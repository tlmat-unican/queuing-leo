#!/usr/bin/env python
# encoding: utf-8

import itertools as it
import json
import numpy as np
import os
import subprocess
import common as common

os.system('./waf --mode=release')

configs = {
    'pktL1000': {
        'elevs': [80],
        'mus': [0.625, 0.0625, 0.00625]
    },
    # 'pktL100': {
    #   'elevs': [40, 60, 80],
    #   'mus': [6.25, 0.625, 0.0625]
    # }
}


steps = 19
npkts = 1e4
iters = 100


for conf, confV in configs.items():
    print(confV)
    lambdaMin = confV['mus'][2]/ 10
    lambdaMax = confV['mus'][2]
    aux = (lambdaMax - lambdaMin) / (steps)
    lamdas = np.arange(lambdaMin, lambdaMax + aux, aux)
    print(lamdas)
    exit(0)
    os.system('rm -rf ./results/{}/*'.format(conf))
    lamdas = [0.003125]
    mus = confV['mus']
    for elev in confV['elevs']:
        jsonFile = './config/satLeo/conf_{}_suburban_bradford_ring_6.json'.format(
                                                                                 elev)
        program = './programs/release/main'

        data = common.ReadJson(jsonFile)
        common.ChangeRateDefault(data, max(mus))
        common.ChangeRates(data, 1, mus)
        common.ChangeRates(data, 7, mus)
        
        
        data['Results']['ResultsPath'] = 'results/{}'.format(conf)
        data['Simulation']['Type'] = 'TRAFFIC_LIMITED'
        # common.ChangePackets(data, 1, npkts)
        common.ChangePacketsDefault(data, npkts)
        

        ctr = 1
        for lamda in lamdas:
            for iter in np.arange(1, iters + 1, 1):
                print('Packet length {}; elevation {}; Lambda {}; iter {}'.format(
                      conf, elev, lamda, iter))
                common.ChangeLambdaDefault(data, lamda)
                data['Results']['PacketsMetrics'] = 'pktMet_elev_{}_lambda_{:03d}.dat'.format(elev, ctr)
                # data['Results']['PacketsNodesTimes'] = 'pktsNodes_elev_{}_lambda_{:03d}.dat'.format(elev, ctr)
                #data['Results']['NodesMetrics'] = 'nodes_elev_{}_lambda_{:06d}'.format(elev, ctr)
                jsonFileOut = './config/satLeo/aux_simple.json'
                common.WriteJson(jsonFileOut, data)
                cmd = '{} {}'.format(program, jsonFileOut)
                process = subprocess.Popen(cmd, shell=True)
                process.wait()
            ctr = ctr + 1
