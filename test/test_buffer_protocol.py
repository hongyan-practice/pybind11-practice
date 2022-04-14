#!/usr/bin/env python
#  -*- coding: utf-8 -*-
__date__ = '2022/4/15'

"""
使用 pybind11 生成并返回 dataframe 的引用
"""
from dist import demo

class TestNumpy:
    def test_buffer_protocol(self):
        tick = demo.Tick()

        arr = tick.get()
        df = tick.get_df()
        print(df)
        tick.wait_update()
        print(df)
        tick.wait_update()
        print(df)