var express = require('express');
var app = express();

const Influx = require('influx');
const influx = new Influx.InfluxDB({
	host: 'localhost',
	database: 'kobold'
	// schema: [
	// {
	// 	measurement: 'response_times',
	// 	fields: {
	// 		path: Influx.FieldType.STRING,
	// 		duration: Influx.FieldType.INTEGER
	// 	},
	// 	tags: [
	// 	'host'
	// 	]
	// }
	// ]
})

app.get('/', function(req, res) {
	influx.query('SELECT * FROM temperature').then(result => {
			// res.json(result)
			res.send(result)
		}).catch(err => {
			res.status(500).send(err.stack)
		})
	});

app.listen(3000);