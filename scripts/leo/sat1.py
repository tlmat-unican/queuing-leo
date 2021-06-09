#!/usr/bin/env python
# encoding: utf-8

import os
import itertools as it
import numpy as np
import json
import subprocess
import common as common
import math

mode = 'release'
os.system('./waf --mode={}'.format(mode))
program = './programs/{}/main'.format(mode)

conf = {
  'elev': 80,
  'mus': [0.625, 0.0625, 0.00625] # packet length 1000 bytes
}


npkts = 1e6
iters =1

# steps = 20
# lambdaMin = 0.0000625
# lambdaMax = 0.05
mus = conf['mus']
# aux = (lambdaMax - lambdaMin) / (steps)
# lamdas = np.arange(lambdaMin, lambdaMax + aux, aux)
# lamdas = np.logspace(math.log10(lambdaMin), math.log10(lambdaMax), 20)
# lamdas = [0.005, 0.01]
lamdas = [0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.010, 0.011, 0.012, 0.013, 0.014, 0.015, 0.016, 0.017, 0.018, 0.019, 0.020]
# print(lamdas)
# exit(0)

elev = conf['elev']
jsonFile = './config/satLeo/conf_{}_suburban_bradford_ring_1.json'.format(
  elev)

data = common.ReadJson(jsonFile)

resultsPath = 'results/1Node_var'        
data['Results']['ResultsPath'] = resultsPath
os.system('rm -rf ./{}/*'.format(resultsPath))

data['Simulation']['Type'] = 'TRAFFIC_LIMITED'
#data['Simulation']['MaxSimulTime'] = 100
data['Simulation']['DefaultFlow']['MaxPackets'] = npkts

ctr = 1
for lamda in lamdas:
  for iter in np.arange(1, iters+1, 1):
    print('Packet length {}; elevation {}; Lambda {} ({}); iter {}'.format(
      conf, elev, lamda, ctr, iter))
    common.ChangeLambdaDefault(data, lamda)
    common.ChangeRates(data, 1, mus)
    # data['Results']['PacketsMetrics'] = 'pktMet_elev_{}_lambda_{:03d}.dat'.format(elev, ctr)
    data['Results']['PacketsTrace'] = 'pktTrace_elev_{}_lambda_{:03d}_iter_{:03d}.dat'.format(elev, ctr, iter)

    data['Results']['NodesMetrics'] = 'nodeMet_elev_{}_lambda_{:03d}'.format(elev, ctr)
    jsonFileOut = './config/satLeo/auxSat1.json'
    common.WriteJson(jsonFileOut, data)
    cmd = '{} {}'.format(program, jsonFileOut)
    process = subprocess.Popen(
      cmd, shell=True)
    process.wait()
  ctr = ctr + 1
