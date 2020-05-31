# gluamysql

## MySQL Lua Connector

Purpose: Create a Garry's Mod Lua MySQL Connector using MariaDB's MySQL C Connector and their async functions

Using this async api and using Lua's lua_yieldk will allow us to create connections to MySQL servers with connector api that handle common errors easily. If a MySQL Server Database connection fails, we can easily reconnect as needed.

## Lua Connector
### This is the API lower level developers can use to design their api around.


#### `gluamysql.connect(host, user, password, db, port)` -> `Promise<gluamysql::Database>`