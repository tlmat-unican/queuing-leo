#!/usr/bin/env python
# encoding: utf-8

import os
import itertools
import numpy as np
import json
import subprocess
import jsonUtils as jsu
import math

mode = 'release'
os.system('./waf --mode={}'.format(mode))
program = './programs/{}/main'.format(mode)

npkts = 1e6
iters =100

muLos = 100
muMs = 50
muDss = [0, 20]
lamdas = [60]
queues = [25, 75]

jsonFile = './config/leoGlobecom/conf_kaband.json'
data = jsu.ReadJson(jsonFile)

resultsPath = 'results/leoGlobecom/kaband'        
data['Results']['ResultsPath'] = resultsPath
os.system('rm -rf ./{}/*'.format(resultsPath))

data['Simulation']['Type'] = 'TRAFFIC_LIMITED'
data['Simulation']['DefaultFlow']['MaxPackets'] = npkts

for lamda, muDs, queue in itertools.product(lamdas, muDss, queues):
  for it in np.arange(1, iters+1, 1):
    mus = [muLos, muMs, muDs]
    jsu.ChangeLambdaDefault(data, lamda)
    jsu.ChangeRates(data, 1, mus)
    jsu.ChangeQueue(data, 1, queue)
    resTail = 'lambda_{:03d}_muds_{:03d}_queue_{:03d}'.format(int(lamda*10), int(muDs*10), queue)
    data['Results']['PacketsMetrics'] = 'pktMet_{}'.format(resTail)
    # data['Results']['PacketsTrace'] = 'pktTrace_{}.dat'.format(resTail)
    # data['Results']['NodesMetrics'] = 'nodeMet_{}'.format(resTail)
    jsonFileOut = './config/leoGlobecom/aux_kaband.json'
    jsu.WriteJson(jsonFileOut, data)
    cmd = '{} {}'.format(program, jsonFileOut)
    os.system(cmd)