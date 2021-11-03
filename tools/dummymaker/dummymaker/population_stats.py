import csv
import random
from collections import namedtuple

JAPAN_POPULATION = 126933000
Record = namedtuple('Record', ['gender', 'age'])


class PopulationStats(object):
    def __init__(self):
        with open('resources/japan_patient_stats.tsv', 'r') as f:
            patient_stats = csv.DictReader(f, dialect='excel-tab')
            self.patient_stats = {}
            for age_range in patient_stats:
                ages = age_range['age'].split('~')
                start_age, end_age = int(ages[0]), int(ages[1])
                for age in range(start_age, end_age + 1):
                    patient_age = {
                        'male': int(age_range['gmal']),
                        'female': int(age_range['gfem'])
                    }
                    # Nyuin should also be included in Gailai I think?
                    self.patient_stats[str(age)] = patient_age

        with open('resources/japan_population_stats.tsv', 'r') as f:
            self.pop_stats = csv.DictReader(f, dialect='excel-tab')
            self.patient_pop = []
            pop_stats_population = 0
            for population in self.pop_stats:
                pop_stats_population += int(population['male'])
                pop_stats_population += int(population['female'])

            f.seek(0)
            self.pop_stats = csv.DictReader(f, dialect='excel-tab')
            # Row back to the front of file
            for population in self.pop_stats:
                male_pop = JAPAN_POPULATION * int(
                    population['male']) / pop_stats_population
                female_pop = JAPAN_POPULATION * int(
                    population['female']) / pop_stats_population
                male_patient = male_pop * self.patient_stats[
                    population['age']]['male'] / 100000
                female_patient = female_pop * self.patient_stats[
                    population['age']]['female'] / 100000
                for _ in range(int(male_patient)):
                    self.patient_pop.append(Record('male', population['age']))
                for _ in range(int(female_patient)):
                    self.patient_pop.append(Record('female',
                                                   population['age']))

        self.pointer_pos = 0
        self.used_id = set()

    def get_res_line(self):
        if self.pointer_pos == 0:
            random.shuffle(self.patient_pop)
            self.pointer_pos = len(self.patient_pop)

        id = random.randint(1, JAPAN_POPULATION)
        while id in self.used_id:
            id = random.randint(1, JAPAN_POPULATION)
        self.used_id.add(id)

        self.pointer_pos -= 1
        return id, self.patient_pop[self.pointer_pos]
