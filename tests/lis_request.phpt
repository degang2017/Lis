--TEST--
Check for LisRequest
--SKIPIF--
<?php if (!extension_loaded("lis")) print "skip"; ?>
--INI--
lis.use_namespace=1
--FILE--
<?php 

if (!file_exists(dirname(__FILE__)."/controller/")) {
    mkdir(dirname(__FILE__)."/controller/");
}
file_put_contents(dirname(__FILE__). "/controller/HomeController.php", <<<PHP
<?php
    class HomeController {
        public static function indexAction() {
            echo "home_index";

            var_dump(\Lis\Request::getMethod());
            var_dump(\Lis\Request::getQuery());
            var_dump(\Lis\Request::getControllerName());
            var_dump(\Lis\Request::getActionName());
        }
    }
PHP
);

\Lis\Application::setAppDirectory(dirname(__FILE__));
$app = new \Lis\Application();

\Lis\Request::setBaseUri('/user/33333/sex3333/33333/ewfwefre/name1/dsdssdds/reset-password');

var_dump(\Lis\Request::getMethod());

\Lis\Route::group('/user/{id_3-3:[0-9]+}/sex3333/{sex:[0-9]+}/ewfwefre', function(){
    \Lis\Route::cli('/name1/{name:[a-zA-Z]+}/reset-password', 'HomeController:indexAction');
});
$app->run();

?>
--EXPECTF--
string(3) "Cli"
home_indexstring(3) "Cli"
array(3) {
  ["id_3-3"]=>
  string(5) "33333"
  ["sex"]=>
  string(5) "33333"
  ["name"]=>
  string(8) "dsdssdds"
}
string(14) "HomeController"
string(11) "indexAction"
