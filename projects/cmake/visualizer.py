#!/usr/bin/python

import lldb

#def Test(value, internal_dict):
#    x = value.GetChildMemberWithName("x").GetValue()
#    y = value.GetChildMemberWithName("y").GetValue()
#    return "(x = {0}, y = {1})".format(x, y)

def __lldb_init_module(debugger, internal_dict):
    #debugger.HandleCommand("type summary add -F " + __name__ + ".Test coreVector2")
    debugger.HandleCommand("type summary add --summary-string \"(${var.x}, ${var.y})\"                     coreVector2")
    debugger.HandleCommand("type summary add --summary-string \"(${var.x}, ${var.y}, ${var.z})\"           coreVector3")
    debugger.HandleCommand("type summary add --summary-string \"(${var.x}, ${var.y}, ${var.z}, ${var.w})\" coreVector4")