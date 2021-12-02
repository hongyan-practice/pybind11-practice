#!/usr/bin/env python
#  -*- coding: utf-8 -*-
__date__ = '2021/6/10'
__author__ = 'hongyan'

import pytest
"""
执行所有的测试用例
"""
if __name__ == "__main__":
    pytest.main(["-s", "test/test_helloworld.py"])
