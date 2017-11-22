var express = require('express');
var app = express();

const Influx = require('influx');
const influx = new Influx.InfluxDB({
	host: 'localhost',
	database: 'kobold'
});

app.set('view engine', 'ejs')

app.get('/', function(req, res) {
	influx.query('SELECT * FROM readings').then(result => {
		res.render('index.ejs');
	}).catch(err => {
		res.status(500).send(err.stack);
	})
});

app.listen(3000);