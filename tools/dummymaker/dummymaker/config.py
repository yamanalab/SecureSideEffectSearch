import random
import numpy
import json


class Distribution(object):
    def __init__(self, conf):
        self.N = conf.get('N')
        self.a = conf.get('a')
        assert self.N and self.a

    def get_problist(self, upper_limit):
        count = random.randint(1, upper_limit)
        sampling = numpy.random.pareto(self.a, count)
        problist = []
        for sample in sampling:
            number = int(sample)
            if number < self.N and number not in problist:
                problist.append(number)

        return problist


class Config(object):
    def __init__(self):
        with open('resources/config.json', 'r') as f:
            fileconf = json.load(f)
            med_dist_conf = fileconf.get('medDistribution')
            side_dist_conf = fileconf.get('sideDistribution')
            assert med_dist_conf is not None
            assert side_dist_conf is not None
            self.med_dist = Distribution(med_dist_conf)
            self.side_dist = Distribution(side_dist_conf)
            self.required_count = fileconf.get('requiredRecordCount', 1000)
            assert self.required_count < 100000009
            self.write_method = fileconf.get('writeForm', 'csv')
            # I hope nobody is going to do such crazy things...
