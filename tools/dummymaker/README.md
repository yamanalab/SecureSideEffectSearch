# Dummy Data Creator for SecureSideEffectSearch
This is a maker of dummy data for SecureSideEffectSearch system.

## Data Structure
Assumption based on [Japan Official Stats Webpage](http://www.stat.go.jp/data/nihon/02.html):

- Gender, Age of current population in Japan drinking medicine:  
Calculated by patient distribution and population distribution.  
Data aggregated into [population stats](resources/japan_population_stats.tsv) and [patient stats](resources/japan_patient_stats.tsv).

- Medicine drinking distribution:  
We can't acquire medicine usage data anywhere.  
We did some assumptions that the usage is following the power-law distribution or 80-20 law ([Paleto II or Lomax Distribution](https://en.wikipedia.org/wiki/Lomax_distribution)).  
<img src="https://docs.scipy.org/doc/numpy-1.13.0/_images/math/a9c9188b69aa00d2c433b0d112382e67dfbc39ed.png" title="lomax" />  

- Side Effect distribution:  
Same as Medicine drinking distribution.  

## Prerequisities

- Python 3.7 or above
- [Poetry](https://github.com/python-poetry/poetry)

## Install dependencies
```
poetry install
```

## Usage
```
poetry run python -m dummymaker
```

The output is saved as `data.csv` or `data.json` based on the configurations of `resources/config.json`

## License

```
Copyright 2020 Yamana Laboratory, Waseda University Supported by JST CREST Grant Number JPMJCR1503, Japan.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
```
