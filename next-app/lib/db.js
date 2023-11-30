const PouchDB = require('pouchdb')

const db = new PouchDB('readings')

module.exports = db