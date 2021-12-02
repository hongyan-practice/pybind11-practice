#!/usr/bin/env python
#  -*- coding: utf-8 -*-
__date__ = '2021/6/10'

from dist import demo

class TestHelloWorld:
    def test_hello_world(self):
        assert demo.hello_world() == 'hello world!'