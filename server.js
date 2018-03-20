var express = require('express');
var app = express();
var bodyParser = require('body-parser');

const Influx = require('influx');
const influx = new Influx.InfluxDB({
	host: 'localhost',
	database: 'kobold'
});

app.set('view engine', 'ejs')

app.get('/', function(req, res) {
	influx.query('SELECT * FROM readings').then(result => {
		res.render('index.ejs', {
			data: result
		});
	}).catch(err => {
		res.status(500).send(err.stack);
	})
});

app.use(bodyParser.json());

app.post('/', function(req, res) {
	console.log(req.body);

	influx.writePoints([
	{
		measurement: 'readings',
		tags: { sensor: req.body.sensor },
		fields: {
			temperature: req.body.temperature,
			voltage:     req.body.voltage
		}
	}]);

	res.status(200).send();
});

app.listen(3000);
