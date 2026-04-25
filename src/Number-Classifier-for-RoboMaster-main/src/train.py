import os
import math
import cv2
import numpy as np
import torch
from torch import nn as nn
from torch import optim
from torch.utils.data import DataLoader, random_split

import onnx
from onnxsim import simplify

from dataset import TensorDataset
from model import Zenet, Lenet5

device = torch.device("cuda")


def loss(pred, label):
    obj_loss_function = nn.BCEWithLogitsLoss()
    cls_loss_function = nn.CrossEntropyLoss()
    gt_obj = label[:, :, 0]
    gt_cls = label[:, :, 1][:, 0].type(torch.LongTensor)
    gt_cls = gt_cls.to(device)

    obj_loss = torch.mean(obj_loss_function((pred[:, :1]), gt_obj))
    cls_loss = torch.mean(cls_loss_function(pred[:, 1:], gt_cls)*gt_obj)

    return 10*obj_loss+5*cls_loss


def train(net, epochs, ds, optimizer, batch_size):
    # 划分训练集和测试集
    train_ratio = 0.8
    train_size = int(train_ratio * len(ds))
    val_size = len(ds) - train_size
    train_dataset, val_dataset = random_split(ds, [train_size, val_size])
    train_dataloader = DataLoader(train_dataset,
                                  batch_size=batch_size,
                                  shuffle=True,
                                  num_workers=4,
                                  drop_last=True,
                                  persistent_workers=True
                                  )
    val_dataloader = DataLoader(val_dataset,
                                batch_size=batch_size,
                                shuffle=True,
                                num_workers=4,
                                drop_last=True,
                                persistent_workers=True
                                )
    best_acc, best_epoch = 0, 0
    print("Training start!")
    # 迭代训练
    for epoch in range(epochs):
        # 训练过程
        net.train()
        for imgs, targets in train_dataloader:
            # 数据预处理
            imgs = imgs.to(device).float() / 255.0
            targets = targets.to(device)
            optimizer.zero_grad()
            output = net(imgs)
            l = loss(output, targets)
            print("Epoch{}: Loss = {}".format(epoch, l))
            l.backward()
            optimizer.step()
        # 测试过程
        net.eval()
        total, correct, positive = 0, 0, 0
        for imgs, targets in val_dataloader:
            imgs = imgs.to(device).float() / 255.0
            targets = targets.to(device)
            output = net(imgs)
            # 获得预测结果向量
            pred = torch.argmax(output[:, 1:], dim=1)
            # 标签向量
            label = targets[:, :, 1:].reshape(batch_size)
            # 计算相等元素(预测正确)的个数
            correct += torch.sum(torch.eq(pred, label))
            total += batch_size
        acc = correct / total
        print("{}\n Epoch{}: Accuracy rate = {}\n{}".format(
            '-'*20, epoch, acc, '-'*20))
        if acc > best_acc:
            best_acc = acc
            best_epoch = epoch
            torch.save(net.state_dict(), "../output/best.pth")
            print("权重已保存")
    return best_acc, best_epoch        



def exportToONNX(net):
    net.eval()
    net.to(torch.device('cpu'))
    dummy_input = torch.randn(1, 1, 28, 28, requires_grad=True)
    torch.onnx.export(net, dummy_input, "../output/Zenet.onnx",
                      export_params=True, opset_version=12, do_constant_folding=True)
    print("导出为Zenet.onnx")                  
    onnx_model = onnx.load("../output/Zenet.onnx")  
    model_sim, check = simplify(onnx_model)
    assert check, "Simplified ONNX model could not be validated"
    print("导出为Zenet-sim.onnx")
    onnx.save(model_sim, "../output/Zenet-sim.onnx")


def initWeights(m):
    if type(m) == nn.Linear or type(m) == nn.Conv2d:
        nn.init.xavier_uniform_(m.weight)


def main():
    cfg_path = "../dataset/armors.txt"
    dataset = train_dataset = TensorDataset(cfg_path, 28, 28, aug=True)

    # net = Zenet(11)
    net = Lenet5
    net.apply(initWeights)
    net.to(device)
    optimizer = optim.Adam(net.parameters(), lr=5e-4)

    epochs = 80
    batch_size = 512
    acc, epc = train(net, epochs, dataset, optimizer, batch_size)
    print("训练结束,最好结果:Epoch:{}, Acc:{}".format(epc, acc))
    if acc > 0.9:
      exportToONNX(net)

if __name__ == '__main__':
  main()      
