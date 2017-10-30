--TEST--
Check for LisContainer
--SKIPIF--
<?php if (!extension_loaded("lis")) print "skip"; ?>
--INI--
lis.use_namespace=1
--FILE--
<?php 
class test {
    public $name;

    public function __construct() {
        echo "test \n";
    }
}
$test = new test();

\Lis\Application::setAppDirectory(dirname(__FILE__));
$app = new \Lis\Application();
\Lis\Container::get();
var_dump(\Lis\Container::add("test", $test));
var_dump(\Lis\Container::get("test"));
$app->run();

?>
--EXPECTF--
test 
bool(true)
object(test)#1 (1) {
  ["name"]=>
  NULL
}
