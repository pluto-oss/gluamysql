# gluamysql

## MySQL Lua Connector

Objective:

Create a Garry's Mod Lua MySQL Connector using MariaDB's MySQL C Connector and their async functions

Purpose:

I've had many problems and many issues with other MySQL modules for Garry's Mod, either infinite looping or unmaintainable code that's been long gone. I want to create a simple to understand and lightweight module for people to developer with for their own needs.

Differences to similar software:

This software will always have a consistent API as long as the MySQL C Connector API is the same, other modules have wrappers upon the C Connector, causing issues and potentially nightmare-ish upgrade paths if their API needs to change.

Using MariaDB's MySQL C Connector is desirable as it supports 32 bit, which most linux Garry's Mod servers will do for a long while as older hosts slowly migrate to the new version, and it also will support 64 bit at the same time!

Using a Promise library, you can potentially use coroutine wrappers to act as if it is sync and have threads dedicated to specific databases; we've created a wrapper ourselves in [cmysql.lua](lua/cmysql.lua)

This is intended to be useful for developers wanting to make a wrapper that suits their own needs such as recreating the API for other MySQL clients built for Garry's Mod

## Lua Connector
### This is the API lower level developers can use to design their api around.
#### Requires [promise.lua](lua/promise.lua)


### Noteworthy Files
#### [cmysql.lua](lua/cmysql.lua) - a simplified version of the c connector api that runs in coroutines
#### [example.lua](lua/example.lua) - an example of how to use gluamysql



### API

#### `gluamysql.connect`(`host`, `user`, `password`, `db`, `port`) -> `Promise<gluamysql::LuaDatabase>`
##### mysql_init, mysql_connect_real

#### `gluamysql::LuaDatabase`:`query`(`querystring`) -> `Promise<table>`
##### mysql_query
Returned table has tables pertaining to how many rows were returned from the query. Data in each row have key as the field name and data in the value.

#### `gluamysql::LuaDatabase`:`prepare`(`querystring`) -> `Promise<gluamysql::LuaPreparedStatement>`
#### mysql_stmt_init, mysql_stmt_prepare

#### `gluamysql::LuaDatabase`:`autocommit`(`autocommit_state`) -> `Promise<>`
##### mysql_autocommit

#### `gluamysql::LuaDatabase`:`commit`() -> `Promise<>`
##### mysql_commit

#### `gluamysql::LuaDatabase`:`rollback`() -> `Promise<>`
##### mysql_rollback

#### `gluamysql::LuaPreparedStatement`:`parametercount`() -> number
##### mysql_stmt_param_count

#### `gluamysql::LuaPreparedStatement`:`execute`(`...`) -> `Promise<table>`
##### mysql_stmt_execute
Similar to `gluamysql::LuaDatabase`:`query`