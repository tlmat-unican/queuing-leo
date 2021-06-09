#!/usr/bin/env python
# encoding: utf-8

import os
import itertools as it
import numpy as np
import json
import pandas as pd

os.system('./waf --mode=release')


def ReadJson(fn):
    with open(fn) as json_file:
        data = json.load(json_file)
        return data


def WriteJson(fn, data):
    with open(fn, 'w') as json_file:
        json.dump(data, json_file)


def AddColumn(fn, val):
    df = pd.read_csv(fn, sep='\t', header=None)
    df.insert(loc=0, column='idx', value=val)
    df.to_csv(fn, sep='\t', index=False, header=None)


def ChangeGamma(data, nodeId, newVal):
    data['Topology']['Node{:01d}'.format(nodeId)]['Regime']['StopParam'] = [
        newVal]


def ChangeLambda(data, val):
    data['Traffic']['DefaultFlow']['Param'] = val


def ChangeFwdProb(data, src, val):
    data['Topology']['ForwardRules'][src] = val


jsonFile = './config/QueueingLeo/twoSat.json'
program = './programs/release/main'
cmd = '{} {}'.format(program, jsonFile)

probs = np.arange(0, 1.1, 0.1)
gammas = np.arange(0, 11, 2)
lamdas = np.arange(0.1, 0.2, 0.1)

data = ReadJson(jsonFile)
data['Simulation']['ResultsPath'] = 'results/QueueingLeo'
os.system('rm -rf {}'.format(data['Simulation']['ResultsPath']))
data['Traffic']['DefaultFlow']['MaxPackets'] = 1e5

for prob, gamma, lamda in it.product(probs, gammas, lamdas):
    ChangeFwdProb(data, 1, [0, 0, 0, prob, 1 - prob, 0])
    ChangeLambda(data, lamda)
    ChangeGamma(data, 3, gamma)
    data['Simulation']['TrafficGlobalMetrics'] = 'probSweep_gamma{:03d}.dat'.format(
        int(100*gamma))
    WriteJson(jsonFile, data)
    print ('Prob {}; gamma {}; lambda {}'.format(prob, gamma, lamda))
    os.system(cmd)

print ('Adding first column')
for gamma in gammas:
    data['Simulation']['TrafficGlobalMetrics'] = 'probSweep_gamma{:03d}.dat'.format(
        int(100*gamma))
    AddColumn('{}/{}'.format(data['Simulation']['ResultsPath'],
                             data['Simulation']['TrafficGlobalMetrics']), probs)
