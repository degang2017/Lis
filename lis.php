<?php
//禁用错误报告
ini_set("display_errors", "On");
////报告运行时错误
//error_reporting(E_ERROR | E_WARNING | E_PARSE);
////报告所有错误
error_reporting(E_ALL);
$br = (php_sapi_name() == "cli")? "":"<br>";
//ini_set("use_namespace", "1");

//dl('./modules/lis.so');


/**
class ExceptionHandler {   
    public static function printException(Exception $e)
    {
        print 'Uncaught '.get_class($e).', code: ' . $e->getCode() . "<br />Message: " . htmlentities($e->getMessage())."\n";
    }
   
    public static function handleException(Exception $e)
    {
         self::printException($e);
    }
}

**/

class t {
    public $a = 'xxxx';

    public function __construct() {
        $this->a = "6666666";
        echo 11111133338888;
    }

    public function test() {
        echo $this->a;
        echo 1113333333;
    }
}
$ttt = new t();

$cons['a'] = $ttt;

LisApplication::setAppDirectory(dirname(__FILE__));

//LisException::setExceptionHandler(array("ErrorController", "handleAction"));
//throw new Exception("Catch me twice", 2);

try{
//echo LIS_VERSION;
$c = new LisApplication($cons);
//var_dump($c);
//
var_dump(LisConfig::set("db", [1 => 4444, 'dddd' => 333333]));
var_dump(LisConfig::set("ss", [1 => ['ewfwefwe' => [3=>2], 4444], 'dddd' => 333333]));
var_dump(LisConfig::get("db.dddd"));
var_dump(LisConfig::get("ss.1.ewfwefwe.3"));
}catch(Exception $e) {
    print_r($e);
}


//LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');

var_dump(LisRequest::getBaseUri());
var_dump(LisRequest::getControllerName());

/**
$b = LisApplication::getEnviron();
$dd = LisContainer::get("a");
var_dump($dd);
$ddc = LisContainer::get();
var_dump($ddc);


class c {
        public $c = "aaa";
     public function __construct(){
        echo "construct ccccc\n";
    }
}

$cccc = new c();
var_dump(LisContainer::add("cccc", $cccc));
$aaaaa = LisContainer::get("cccc");
var_dump($aaaaa);
$ddc = LisContainer::get();
var_dump($ddc);




$c = new UserModel();
try{
    $c->test();
}catch (Exception $e) {
    echo "xxxxx";
    var_dump($e);
}
**/
/**
$ddd = LisApplication::getAppDirectory();
var_dump($dd);
var_dump($ddd);
//var_dump($b);
//var_dump($c);
$ccc = LisContainer::get();
var_dump($ccc);
die;


//$c = new LisApplication();
//var_dump($c);
//echo ini_get('lis.environ');
//echo "$str\n";
echo "----\n";
//$b = $c->a->test();
//var_dump($b);

echo 11111;

echo 111133333;
echo '---------------------------'."\n";
//$container = LisApplication::$container;
//var_dump($container);
//var_dump($container['config']);
echo '---------------------------'."\n";

//var_dump($c->a->test());
//var_dump($c->config->init());
//var_dump($c->exception);

//var_dump($c->config->set("db", [1 => 4444, 'dddd' => 333333]));
//var_dump($c->config->set("ss", [1 => ['ewfwefwe' => [3=>2], 4444], 'dddd' => 333333]));
//var_dump($c->config->get("db.dddd"));
//var_dump($c->config->get("ss.1.ewfwefwe.3"));
echo '---------------------------'."\n";

Lis_Config::set("db", [ 1 => 4444, 'dddd' => 3333]);
var_dump(Lis_Config::get("db.1"));

**/
echo '---------config------------------'."\n";
LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');


LisRoute::group('/xuserx/{id_3-3:[0-9]+}/sex3333/{sex:[0-9]+}/ewfwefre', function(){
    LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
    LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
    LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
});

LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');

LisRoute::group('/userx/{id_3-3:[0-9]+}/sex3333/{sex:[0-9]+}/ewfwefre', function(){
    LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
    LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
    LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
});

LisRoute::group('/user/{id_3-3:[0-9]+}/sex3333/{sex:[0-9]+}/ewfwefre', function(){
    LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
    LisRoute::get('/name1/{name:[a-zA-Z]+}/reset-password', 'HomeController:testAction');
    //LisApplication::get('/name3/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
});
echo "--------request0-----\n";

LisRoute::group('/userx/{id_3-3:[0-9]+}/sex3333/{sex:[0-9]+}/ewfwefre', function(){
    LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
    LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
    LisRoute::get('/name2/{name:[a-zA-Z]+}/reset-password', 'HomeController:home');
});

/**
LisApplication::group('/user/ewfwefre', function(){
  LisApplication::get('/name2', 'HomeController:testAction');
});

var_dump(Lis_Request::$_controller_name);
var_dump(Lis_Request::$_action_name);

$c = new UserModel();
$c->test();
**/
/**
class a {

    public function group($name, $func) {
        echo $name."\n";
        $b = 333;
        call_user_func($func, $b);
        return 'ewfew';
    }
}

$a = new a();

$c = $a->group('name', function ($arg) {
    var_dump($arg);
    echo 'xxxxxxxx';
});
var_dump($c);


echo "----group \n";
$c = Lis_Request::$_base_uri;
var_dump($c);

//$a = Lis_Router::$_group_route;
//var_dump($a);
$c = new UserModel();
UserModel::test();;
**/
/**
$bbb = Lis_Request::$_base_uri;
echo "-------\n";
var_dump($bbb);
Lis_Loader::import(dirname(__FILE__)."/a/Hello.php");
Hello::test();

Lis_Loader::import(dirname(__FILE__)."/a/Hello.php");
Hello::test();
**/

?>
