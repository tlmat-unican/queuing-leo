#!/usr/bin/env python
# encoding: utf-8
import json
import numpy as np

def ReadJson(fn):
    with open(fn) as json_file:
        data = json.load(json_file)
        return data

def WriteJson(fn, data):
    with open(fn, 'w') as json_file:
        json.dump(data, json_file)

## These are to change flows configurations
def ChangeLambda(js, flowId, val):
    js['Simulation']['Flow{}'.format(flowId)]['Param'] = val

def ChangePackets(js, flowId, val):
    js['Simulation']['Flow{}'.format(flowId)]['MaxPackets'] = val    

def ChangeLambdaDefault(js, val):
    js['Simulation']['DefaultFlow']['Param'] = val

def ChangePacketsDefault(js, val):
    js['Simulation']['DefaultFlow']['MaxPackets'] = val        

## These are to change nodes configuration
def ChangeRates(js, nodeId, val):
    js['Topology']['Node{}'.format(nodeId)]['Service']['Param'] = val

def ChangeRateDefault(js, val):
    js['Topology']['DefaultNode']['Param'] = val

def ChangeRegimeProbs(js, nodeId, val):
    js['Topology']['Node{}'.format(nodeId)]['Regime']['Prob'] = val

def ChangeRegimeChangeRate(js, nodeId, val):
    js['Topology']['Node{}'.format(nodeId)]['Regime']['Param'] = val

def AddForwardMat(js, flowId, nnodes, rules):
  m = np.zeros((nnodes, nnodes))
  for rule in rules:
    m[rule[0]][rule[1]] = rule[2]
  
  for row in m:
    aux = np.sum(row)
    row[0] = row[0] + (1-aux)
  js['Topology']['ForwardRules']['Flow{}'.format(flowId)] = m.tolist()

def AddForwardPath(js, flowId, nnodes, path):
  rules = []
  for idx in range (1, len(path)):
    rules.append([path[idx-1], path[idx], 1])
  AddForwardMat(js, flowId, nnodes, rules)

def AddFlow(js, flowId, ftype, param, npkts):
  js['Simulation']['Flow{}'.format(flowId)] = {
    'Dist': ftype,
    'Param': param,
    'MaxPackets': npkts
  }

def AddFlowDefault(js, ftype, param, npkts):
  js['Simulation']['DefaultFlow'] = {
    'Dist': ftype,
    'Param': param,
    'MaxPackets': npkts
  }  

def AddNodeMG1(js, nodeId, ntype, qs, param):
  js['Topology']['Node{}'.format(nodeId)] = {
    'Dist': ntype,
    'QueueSize': qs,
    'Type': 'MM1',
    'Param': param
  }

def AddNodeML(js, nodeId, qs, serv, reg):
  js['Topology']['Node{}'.format(nodeId)] = {
    'QueueSize': qs,
    'Type': 'ML2',
    'Service': serv,
    'Regime': reg
  }