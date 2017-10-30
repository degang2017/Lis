--TEST--
Check for LisView
--SKIPIF--
<?php if (!extension_loaded("lis")) print "skip"; ?>
--INI--
lis.use_namespace=1
--FILE--
<?php 
file_put_contents(dirname(__FILE__). "/view/index.php", <<<PHP
<html>
3333
<?php echo \$var1; ?> 
<?php echo \$var2; ?> 
<html>
PHP
);

\Lis\Application::setAppDirectory(dirname(__FILE__));
$app = new \Lis\Application();

\Lis\View::assign("var1", "hello test111");
\Lis\View::render("index", ["var2" => "test2222"]);

$app->run();

?>
--EXPECTF--
<html>
3333
hello test111 
test2222 
<html>
