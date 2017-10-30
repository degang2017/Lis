--TEST--
Check for LisLoader
--SKIPIF--
<?php if (!extension_loaded("lis")) print "skip"; ?>
--INI--
lis.use_namespace=1
--FILE--
<?php 

if (!file_exists(dirname(__FILE__)."/util/")) {
    mkdir(dirname(__FILE__)."/util/");
}
file_put_contents(dirname(__FILE__). "/util/Hello.php", <<<PHP
<?php
    class Hello {
        public static function test() {
            echo "test";
        }
    }
PHP
);


\Lis\Application::setAppDirectory(dirname(__FILE__));

$app = new \Lis\Application();

\Lis\Loader::import(dirname(__FILE__)."/util/Hello.php");
Hello::test();
$app->run();

?>
--EXPECTF--
test
