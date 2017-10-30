--TEST--
Check for LisException
--SKIPIF--
<?php if (!extension_loaded("lis")) print "skip"; ?>
--INI--
lis.use_namespace=1
--FILE--
<?php 

if (!file_exists(dirname(__FILE__)."/controller/")) {
    mkdir(dirname(__FILE__)."/controller/");
}
file_put_contents(dirname(__FILE__). "/controller/ErrorController.php", <<<PHP
<?php
    class ErrorController {
        public static function handleAction(\$e) {
            var_dump(\$e);
        }
    }
PHP
);

\Lis\Application::setAppDirectory(dirname(__FILE__));
\Lis\Exception::setExceptionHandler(array("ErrorController", "handleAction"));

$app = new \Lis\Application();

try{
    throw new Exception("Catch me", 1);
}catch(Exception $e) {
    echo $e->getMessage();
}

$app->run();

?>
--CLEAN--
<?php
?>
--EXPECTF--
Catch me
