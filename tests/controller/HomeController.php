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