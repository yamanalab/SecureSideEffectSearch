from dummymaker import config
from dummymaker import population_stats
import json
import csv
from datetime import datetime
import numpy as np


class DummyMaker(object):
    def __init__(self):
        self.config = config.Config()
        self.popstats = population_stats.PopulationStats()

    def _run_impl(self):
        dummy = {'records': []}
        # id,medicine_id,symptom_id,symptom_orthographical_variant_id,hit,evaluation,shop_id,patient_id,age,gender,note,created_at
        for rec_id in range(self.config.required_count):
            record = {'id': rec_id + 1}
            record['medicine_id'] = self.config.med_dist.get_problist(20)
            record['patient_id'], gender_age = self.popstats.get_res_line()
            record['gender'] = 1 if gender_age.gender == "male" else 2
            record['age'] = gender_age.age
            record['symptom_id'] = self.config.side_dist.get_problist(5)
            dummy['records'].append(record)

        if self.config.write_method == 'csv':
            with open('data.csv', 'w+') as f:
                w = csv.writer(f, delimiter=",")
                headers = [
                    "id", "medicine_id", "symptom_id",
                    "symptom_orthographical_variant_id", "hit", "evaluation",
                    "shop_id", "patient_id", "age", "gender", "note",
                    "created_at"
                ]
                w.writerow(headers)
                for rec in dummy['records']:
                    for medicine_id in rec['medicine_id']:
                        for symptom_id in rec['symptom_id']:
                            now = datetime.now()
                            w.writerow([
                                str(x) for x in [
                                    rec['id'],
                                    medicine_id + 1,
                                    symptom_id + 1,
                                    "",
                                    # TODO(musaprg): Stop using rand function, and consider based on the frequency of the side-effect
                                    np.random.randint(0, 2),
                                    "",
                                    "myownshop",
                                    rec['patient_id'] + 1,
                                    rec['age'],
                                    rec['gender'],
                                    "",
                                    now.strftime("%Y-%m-%d %H:%M:%S")
                                ]
                            ])

        elif self.config.write_method == 'json':
            with open('data.json', 'w+') as f:
                json.dump(dummy, f, indent=2)

        else:
            raise AttributeError(
                'Please set correct output method. "{}" is not amongst ["csv", "json"].'
                .format(self.config.write_method))


def _main():
    dummy_maker = DummyMaker()
    dummy_maker._run_impl()


if __name__ == '__main__':
    _main()
