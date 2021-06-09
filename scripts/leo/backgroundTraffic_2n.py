#!/usr/bin/env python
# encoding: utf-8

import itertools as it
import json
import numpy as np
import os
import subprocess
import common as common

os.system('./waf --mode=release')

configPktL1000 = {
  'elev': 40,
  'mus': [0.625, 0.0625, 0.00625]
}

npkts = 1e3
iters = 50

mus = configPktL1000['mus']
elev = configPktL1000['elev']
lambdaMain = configPktL1000['mus'][2] / 2

steps = 20
lambdaBgMin = 0.5*max(mus)
lambdaBgMax = 0.9 * max(mus)
lamdasBg = np.linspace(lambdaBgMin, lambdaBgMax, steps)

jsonFile = './config/satLeo/conf_{}_suburban_bradford_ring_6_bg_2.json'.format(elev)
program = './programs/release/main'

data = common.ReadJson(jsonFile)
common.ChangeRateDefault(data, max(mus))
common.ChangeRates(data, 1, mus)
common.ChangeRates(data, 7, mus)
common.ChangeLambda(data, 1, lambdaMain)

resultsPath = 'results/pktL1000_BG2'        
os.system('rm -rf ./{}/*'.format(resultsPath))        
data['Results']['ResultsPath'] = resultsPath

ctr = 1
for lamdaBg in lamdasBg:
  for iter in np.arange(1, iters + 1, 1):
    print('elevation {:02d};  Lambda Bg {:3.7f};  Iter {:04d}'.format(elev, lamdaBg, iter))
    common.ChangeLambda(data, 2, lamdaBg)
    common.ChangeLambda(data, 3, lamdaBg)

    data['Simulation']['Type'] = 'TRAFFIC_LIMITED'
    auxPktsBg = int(1.5 * (lamdaBg / lambdaMain) * npkts)
    print("Packets main {}, packets background {}".format(npkts, auxPktsBg))
    
    common.ChangePackets(data, 1, npkts)
    common.ChangePackets(data, 2, auxPktsBg)
    common.ChangePackets(data, 3, auxPktsBg)
    
    data['Simulation']['NumFlows'] = 3
    data['Results']['PacketsMetrics'] = 'pkts_elev_{}_lambdaBg_{:03d}.dat'.format(elev, ctr)
    # data['Results']['PacketsTrace'] = 'pktsTrace_elev_{}_lambda_{:03d}.dat'.format(elev, ctr)
    # data['Results']['PacketsNodesTimes'] = 'pktsNode_elev_{}_lambda_{:03d}'.format(elev, ctr)
    # data['Results']['NodesMetrics'] = 'nodes_elev_{}_lambda_{:03d}.dat'.format(elev, ctr)
    jsonFileOut = './config/satLeo/auxBG2.json'
    common.WriteJson(jsonFileOut, data)
    cmd = '{} {}'.format(program, jsonFileOut)
    process = subprocess.Popen(cmd, shell=True)
    process.wait()
  ctr = ctr + 1

print(lamdasBg)    