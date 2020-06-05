# gluamysql
## A Garry's Mod Lua MySQL Connector

---
### Objective

Create a Garry's Mod Lua MySQL Connector using MariaDB's MySQL C Connector and their async functions

### Purpose

I've had many problems and many issues with other MySQL modules for Garry's Mod, either infinite looping causing stalls or unmaintainable code that I will never be able to comprehend. This module's aim is to create a simple to understand and lightweight module for people to develop and adapt to their own needs.

### Advantages over similar software

- This will always have a consistent API as long as the MySQL C Connector API is the same.
- Using MariaDB's MySQL C Connector we will be able to support 32 bit and 64 bit servers. Newer Oracle MySQL C Connectors will only support 64 bit.
- The MariaDB Non-blocking library prevents unnecessary complexities from extra threads while losing next to no performance.
- Using a Promise library for every return, you can use coroutines to have your code feel like it's synchronous without having to deal with callbacks; this package includes an example: [cmysql.lua](lua/cmysql.lua)

This is intended to be useful for developers wanting to make a wrapper that suits their own needs such as recreating the API for other MySQL clients built for Garry's Mod

---

### Noteworthy Files
-  [cmysql.lua](lua/cmysql.lua) - a simplified version of the c connector api that runs in coroutines
-  [example.lua](lua/example.lua) - an example of how to use gluamysql

--- 

## Connector API
### This is the API lower level developers can use to design their api around.
- Requires [promise.lua](lua/promise.lua)

---

#### `gluamysql.connect`(`host`, `user`, `password`, `db`, `port`) -> `Promise`<`gluamysql::LuaDatabase`>
**see: `mysql_init`, `mysql_connect_real`**

---

#### `gluamysql::LuaDatabase`:`query`(`querystring`) -> `Promise`<`table`>
**see: `mysql_query`**

Returned table has tables pertaining to how many rows were returned from the query. Data in each row have key as the field name and data as the value.

---

#### `gluamysql::LuaDatabase`:`prepare`(`querystring`) -> `Promise`<`gluamysql::LuaPreparedStatement`>
**see: `mysql_stmt_init`, `mysql_stmt_prepare`**

---

#### `gluamysql::LuaDatabase`:`autocommit`(`autocommit_state`) -> `Promise`<>
**see: `mysql_autocommit`**

---

#### `gluamysql::LuaDatabase`:`commit`() -> `Promise`<>
**see: `mysql_commit`**

---

#### `gluamysql::LuaDatabase`:`rollback`() -> `Promise`<>
**see: `mysql_rollback`**

---

#### `gluamysql::LuaPreparedStatement`:`parametercount`() -> `number`
**see: `mysql_stmt_param_count`**

---

#### `gluamysql::LuaPreparedStatement`:`execute`(`...`) -> `Promise`<`table`>
**see: `mysql_stmt_execute`**
Similar to `gluamysql::LuaDatabase`:`query`

---