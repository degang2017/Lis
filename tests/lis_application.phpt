--TEST--
Check for LisApplication
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
$app = new \Lis\Application(['test' => $test]);
var_dump($app);
$environ = \Lis\Application::getEnviron();
$appDirectory = \Lis\Application::getAppDirectory();
echo \LIS\VERSION."\n";
var_dump($environ, $test);
$app->run();

?>
--EXPECTF--
test 
object(Lis\Application)#2 (2) {
  ["_running":protected]=>
  bool(true)
  ["_environ":protected]=>
  string(7) "develop"
}
1.0.0
string(7) "develop"
object(test)#1 (1) {
  ["name"]=>
  NULL
}
