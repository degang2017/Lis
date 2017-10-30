# Lis
基于C开发高性能PHP框架

```c
LisApplication::setAppDirectory(dirname(__FILE__));

$app = new LisApplication();

LisRoute::get('/name/{name:[a-zA-Z]+}/', 'HomeController:home');

LisRoute::group('/user/{id:[0-9]+}/age/{age:[0-9]+}/', function(){

    LisRoute::get('/sex/{sex:[a-zA-Z]+}/info','HomeController:home');

}))->middleware('Middle:test');

$app->run();
```
#### 网站地址

    http://ulive.me
