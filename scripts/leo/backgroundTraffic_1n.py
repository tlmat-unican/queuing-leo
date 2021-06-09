#!/usr/bin/env python
# encoding: utf-8

import itertools as it
import json
import numpy as np
import os
import subprocess
import common as common
import math

os.system('./waf --mode=release')

configPktL1000 = {
  'elev': [40, 60, 80],
  'mus': [0.625, 0.0625, 0.00625]
}

npkts = 1e5
iters = 1

mus = configPktL1000['mus']
elevs = configPktL1000['elev']
lambdaMain = configPktL1000['mus'][2] / 2

steps = 20
lambdaBgMin = 0.5*max(mus)
lambdaBgMax = 0.9*max(mus)
lamdasBg = [0.5*max(mus), 0.7*max(mus), 0.9*max(mus)]
#np.linspace(lambdaBgMin, lambdaBgMax, steps)

resultsPath = 'results/pktL1000_BG1_all'        
os.system('rm -rf ./{}/*'.format(resultsPath))        
for elev in elevs:
  jsonFile = './config/satLeo/conf_{}_suburban_bradford_ring_6_bg_1.json'.format(elev)
  program = './programs/release/main'

  data = common.ReadJson(jsonFile)
  common.ChangeRateDefault(data, max(mus))
  common.ChangeRates(data, 1, mus)
  common.ChangeRates(data, 7, mus)
  common.ChangeLambda(data, 1, lambdaMain)

  
  data['Results']['ResultsPath'] = resultsPath


  ctr = 1
  for lamdaBg in lamdasBg:
    for iter in np.arange(1, iters + 1, 1):
      print('elevation {:02d};  Lambda Bg {:3.7f};  Iter {:04d}'.format(elev, lamdaBg, iter))
      common.ChangeLambda(data, 2, lamdaBg)

      data['Simulation']['Type'] = 'TRAFFIC_LIMITED'
      auxPktsBg = int(1.1 * (lamdaBg / lambdaMain) * npkts)
      print("Packets main {}, packets background {}".format(npkts, auxPktsBg))

      common.ChangePackets(data, 1, npkts)
      common.ChangePackets(data, 2, auxPktsBg)

      if (auxPktsBg == 0):
        data['Simulation']['NumFlows'] = 1
      else:
        data['Simulation']['NumFlows'] = 2
      # data['Results']['PacketsMetrics'] = 'pkts_elev_{}_lambdaBg_{:03d}.dat'.format(elev, ctr)
      data['Results']['PacketsTrace'] = 'pktsTrace_elev_{}_lambda_{:03d}.dat'.format(elev, ctr)
      # data['Results']['PacketsNodesTimes'] = 'pktsNode_elev_{}_lambda_{:03d}.dat'.format(elev, ctr)
      # data['Results']['NodesMetrics'] = 'nodes_elev_{}_lambda_{:03d}.dat'.format(elev, ctr)
      jsonFileOut = './config/satLeo/auxBG1_all.json'
      common.WriteJson(jsonFileOut, data)
      cmd = '{} {}'.format(program, jsonFileOut)
      process = subprocess.Popen(cmd, shell=True)
      process.wait()
    ctr = ctr + 1

print(lamdasBg)  