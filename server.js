var express = require('express')
var app = express()
var bodyParser = require('body-parser')
var passport = require('passport')
var LocalStrategy = require('passport-local').Strategy
var bcrypt = require('bcrypt')
var moment = require('moment')

const Sequelize = require('sequelize');
const sequelize = new Sequelize('kobold', 'aardvarkk', null, {
  dialect: 'postgres'
});

sequelize
  .authenticate()
  .then(() => {
    console.log('Connection has been established successfully.');
  })
  .catch(err => {
    console.error('Unable to connect to the database:', err);
  })

// MODELS
const User = sequelize.define('user', {
  id: { type: Sequelize.BIGINT, primaryKey: true },
  email: { type: Sequelize.TEXT, allowNull: false },
  password: { type: Sequelize.TEXT, allowNull: false }
}, {
  timestamps: false
})

const Node = sequelize.define('node', {
  id: { type: Sequelize.BIGINT, primaryKey: true },
  name: { type: Sequelize.TEXT, allowNull: false },
  mac_addr: { type: Sequelize.TEXT, allowNull: false }
}, {
  timestamps: false
})

const Override = sequelize.define('override', {
  id: { type: Sequelize.BIGINT, primaryKey: true },
  temp: { type: Sequelize.REAL, allowNull: false },
  ts: { type: Sequelize.DATE }
}, {
  timestamps: false
})

const Reading = sequelize.define('reading', {
  id: { type: Sequelize.BIGINT, primaryKey: true },
  temp: { type: Sequelize.REAL },
  ts: { type: Sequelize.DATE }
}, {
  timestamps: false
})

User.hasMany(Node, { as: 'Nodes', foreignKey: 'user_id' })
Node.belongsTo(User, { foreignKey: 'user_id' })

Node.hasMany(Reading, { as: 'Readings', foreignKey: 'node_id' })
Reading.belongsTo(Node, { foreignKey: 'node_id' })

Node.hasOne(Override, { as: 'Override', foreignKey: 'override_id' })
Override.belongsTo(Node, { foreignKey: 'node_id' })

app.set('view engine', 'ejs')
app.use(bodyParser.json())
app.use(bodyParser.urlencoded())
app.use(passport.initialize());
app.use(passport.session());

passport.use(new LocalStrategy({ usernameField: 'email' },
  function(email, password, done) {
    User.findOne({ where: { email: email }}).then(user => {
      if (!user) return done(null, false)
      bcrypt.compare(password, user.password, function(err, res) {
        if (!res) return done(null, false)
        return done(null, user)
      });
      // bcrypt.hash('password', 10, function(err, hash) {
      //   console.log(hash)
      // });
    })
  }
))

passport.serializeUser(function(user, done) {
  done(null, user.email);
});

passport.deserializeUser(function(email, done) {
  User.findOne({ where: { email: email } }).then(user => {
    done(!user, user);
  });
});

app.get('/login', function(req, res) {
  console.log(req.user)
  res.render('login.ejs')
})

app.post('/login', passport.authenticate('local'), function(req, res) {
  res.redirect('/')
})

app.get('/nodes', function(req, res) {
  var nodeVals = {}
  Node.findAll({ where: { user_id: 1 }})
  .then(result => {
    // console.log(result)
    nodes = result
  })
  .then(() => {
    // TODO: Use actual user ID
    return sequelize.query(`
      SELECT DISTINCT ON (nodes.id)
      nodes.id,
      readings.temp AS reading_temp,
      readings.ts AS reading_ts,
      overrides.temp AS override_temp,
      overrides.ts AS override_ts
      FROM nodes
      JOIN readings ON readings.node_id = nodes.id
      LEFT JOIN overrides ON overrides.node_id = nodes.id
      WHERE user_id = 1
      ORDER BY nodes.id, readings.ts DESC
    `, { type: sequelize.QueryTypes.SELECT })
  })
  .then(results => {
    // console.log(results)
    results.forEach(function(result) {
      nodeVals[result.id] = result
    })
  })
  .then(() => {
    // console.log(nodes)
    // console.log(nodeVals)
    res.render('nodes.ejs', {
      moment: moment,
      nodes: nodes,
      nodeVals: nodeVals
    })
  })
})

app.get('/', function(req, res) {
  // influx.query('SELECT * FROM readings').then(result => {
  //   res.render('index.ejs', {
  //     data: result
  //   })
  // }).catch(err => {
  //   res.status(500).send(err.stack)
  // })
})

app.post('/', function(req, res) {
  console.log(req.body)

  // influx.writePoints([
  // {
  //   measurement: 'readings',
  //   tags: { sensor: req.body.sensor },
  //   fields: {
  //     temperature: req.body.temperature,
  //     voltage:     req.body.voltage
  //   }
  // }])

  res.status(200).send()
})

app.listen(3000)
