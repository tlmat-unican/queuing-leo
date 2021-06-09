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
  'elev': 80,
  'mus': [0.625, 0.0625, 0.00625]
}

jsonFileOut = './config/satLeo/aux.json'
# steps = 20
npkts = 1e6
iters = 1

# lambdaMin = configPktL1000['mus'][2] / 10
# lambdaMax = configPktL1000['mus'][2]*1.5
# aux = (lambdaMax - lambdaMin) / (steps)
# lamdas = np.arange(lambdaMin, lambdaMax + aux, aux)
lamdas = [0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.010]
# lamdas = [lambdaMin]
mus = configPktL1000['mus']

elev = configPktL1000['elev']
jsonFile = './config/satLeo/conf_{}_suburban_bradford_ring_6.json'.format(elev)
program = './programs/release/main'

data = common.ReadJson(jsonFile)
common.ChangeRateDefault(data, max(mus))
common.ChangeRates(data, 1, mus)
common.ChangeRates(data, 7, mus)

resultsPath = 'results/leo/pktL1000_var_lenConst'        
os.system('rm -rf ./{}/*'.format(resultsPath))        
data['Results']['ResultsPath'] = resultsPath

data['Simulation']['Type'] = 'TRAFFIC_LIMITED'
# data['Simulation']['MaxSimulTime'] = 100
data['Simulation']['DefaultFlow']['MaxPackets'] = npkts

ctr = 1
for lamda in lamdas:
  print('elevation {}; Lambda {}'.format(elev, lamda))
  common.ChangeLambdaDefault(data, lamda)
  data['Results']['PacketsTrace'] = 'pktsTrace_elev_{}_lambda_{:03d}.dat'.format(elev, ctr)
  
  common.WriteJson(jsonFileOut, data)
  cmd = '{} {}'.format(program, jsonFileOut)
  process = subprocess.Popen(cmd, shell=True)
  process.wait()
  ctr = ctr + 1