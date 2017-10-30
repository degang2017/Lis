# Lis
基于C开发高性能PHP框架


#### 案例

```c
LisApplication::setAppDirectory(dirname(__FILE__));

$app = new LisApplication();

LisRoute::get('/name/{name:[a-zA-Z]+}/', 'HomeController:home');

LisRoute::group('/user/{id:[0-9]+}/age/{age:[0-9]+}/', function(){

    LisRoute::get('/sex/{sex:[a-zA-Z]+}/info','HomeController:home');

}))->middleware('Middle:test');

$app->run();
```

#### 版本要求

- PHP 7.1.10 +

#### 编译 

```
    $/path/to/phpize
    $./configure --with-php-config=/path/to/php-config
    $make && make install
```

#### 文档 

    https://github.com/degang2017/lis_manual 
