https://docs.influxdata.com/influxdb/v1.3/introduction/getting_started/
https://docs.influxdata.com/influxdb/v1.3/query_language/data_exploration/

```
brew install influxdb
brew services start influxdb
curl "http://localhost:8086/query?q=show+databases"
influx -precision rfc3339
CREATE DATABASE kobold
SHOW DATABASES
USE kobold
INSERT temperature,sensor=287AA6A807000029 value=21.56
curl -i -XPOST 'http://localhost:8086/write?db=kobold' --data-binary 'temperature,sensor=287AA6A807000029 value=21.56'
SELECT * FROM temperature
SELECT * FROM temperature WHERE sensor = '287AA6A807000029'
SELECT * FROM temperature WHERE sensor = '287AA6A807000029' AND time >= now() - 1d
curl -G 'http://localhost:8086/query?pretty=true' --data-urlencode "db=kobold" --data-urlencode "q=SELECT * FROM temperature WHERE sensor = '287AA6A807000029' AND time >= now() - 1d"
exit
```
