--TEST--
Check for LisRoute
--SKIPIF--
<?php if (!extension_loaded("lis")) print "skip"; ?>
--INI--
lis.use_namespace=1
--FILE--
<?php 

\Lis\Application::setAppDirectory(dirname(__FILE__));
$app = new \Lis\Application();
var_dump(\Lis\Route::get('/name/{name:[a-zA-Z]+}/reset-password', 'HomeController:home'));
var_dump(\Lis\Route::post('/name/{name:[a-zA-Z]+}/reset-password', 'HomeController:home'));
var_dump(\Lis\Route::put('/name/{name:[a-zA-Z]+}/reset-password', 'HomeController:home'));
var_dump(\Lis\Route::patch('/name/{name:[a-zA-Z]+}/reset-password', 'HomeController:home'));
var_dump(\Lis\Route::delete('/name/{name:[a-zA-Z]+}/reset-password', 'HomeController:home'));
var_dump(\Lis\Route::options('/name/{name:[a-zA-Z]+}/reset-password', 'HomeController:home'));

var_dump(\Lis\Route::group('/user/{id_3-3:[0-9]+}/age/{age:[0-9]+}/', function(){
    \Lis\Route::get('/name/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
}));

$app->run();
?>
--CLEAN--
<?php
?>
--EXPECTF--
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
