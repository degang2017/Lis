--TEST--
Check for LisConfig
--SKIPIF--
<?php if (!extension_loaded("lis")) print "skip"; ?>
--INI--
lis.use_namespace=1
--FILE--
<?php 

\Lis\Application::setAppDirectory(dirname(__FILE__));

$app = new \Lis\Application();
$dbconfig = ['test_db' => [ 'user' => 'db2' ] ];
\Lis\Config::load($dbconfig);

var_dump(\Lis\Config::set('db', [ 'user' => 'db1', 'passport' => '123']));
var_dump(\lis\Config::get('db'));
var_dump(\lis\Config::get('db.user'));
var_dump(\lis\Config::get('test_db'));
$app->run();


?>
--EXPECTF--
bool(true)
array(2) {
  ["user"]=>
  string(3) "db1"
  ["passport"]=>
  string(3) "123"
}
string(3) "db1"
array(1) {
  ["user"]=>
  string(3) "db2"
}
