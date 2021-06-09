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

muLos = 5
muMs = 2.5
muDss = [0, 1]
lamdas = [2, 3]
# lamdas = [0.5, 1, 2, 3, 4, 5, 6]
queues = [25, 75, 125]
# queues = [7, 15]

jsonFile = './config/leoGlobecom/conf_sband.json'
data = jsu.ReadJson(jsonFile)

resultsPath = 'results/leoGlobecom/sband/scatter'        
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
    jsonFileOut = './config/leoGlobecom/aux_sband_scatter.json'
    print('lamda {};  muDs {}; queue {}; iter {}'.format(lamda, muDs, queue, it))
    jsu.WriteJson(jsonFileOut, data)
    cmd = '{} {}'.format(program, jsonFileOut)
    os.system(cmd)