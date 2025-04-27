#!/usr/bin/python
import lldb


class coreResourceIndexSynthetic:

    def __init__(self, value, dict):
        self.value  = value
        self.table0 = value.target.FindFirstGlobalVariable("coreResourceManager::s_apResourceTable")
        self.table1 = value.target.FindFirstGlobalVariable("coreResourceManager::s_apHandleTable")

    def num_children(self):
        return 2

    def get_child_at_index(self, index):
        if index == 0: return self.value.target.CreateValueFromData("[resource]", self.table0.GetPointeeData(self.value.GetValueAsUnsigned()), self.table0.GetType().GetArrayElementType())
        if index == 1: return self.value.target.CreateValueFromData("[handle]",   self.table1.GetPointeeData(self.value.GetValueAsUnsigned()), self.table1.GetType().GetArrayElementType())


def coreResourcePtrSummary(value, internal_dict):

    index = value.GetChildMemberWithName("m_iIndex").GetValueAsUnsigned()

    if index == 0: return "(null)"

    table   = value.target.FindFirstGlobalVariable("coreResourceManager::s_apHandleTable")
    handle  = value.target.CreateValueFromData("[handle]", table.GetPointeeData(index), table.GetType().GetArrayElementType())
    summary = handle.GetValueForExpressionPath(".m_sName._M_dataplus._M_p").GetSummary()

    if summary == "\"\"": return "(custom)"

    return F"({summary})"


def __lldb_init_module(debugger, internal_dict):

    debugger.HandleCommand("type synthetic add coreResourceIndex            --python-class    " + __name__ + ".coreResourceIndexSynthetic")
    debugger.HandleCommand("type summary   add -x \"^coreResourcePtr<.+>$\" --python-function " + __name__ + ".coreResourcePtrSummary")
    debugger.HandleCommand("type summary   add -x \"^coreList<.+>$\"        --inline-children --omit-names")
    debugger.HandleCommand("type summary   add -x \"^coreSet<.+>$\"         --inline-children --omit-names")
    debugger.HandleCommand("type summary   add -x \"^coreMapGen<.+>$\"      --inline-children --omit-names")
    debugger.HandleCommand("type summary   add -x \"^coreMapStrSlim<.+>$\"  --inline-children --omit-names")
    debugger.HandleCommand("type summary   add -x \"^coreMapStrFull<.+>$\"  --inline-children --omit-names")
    debugger.HandleCommand("type summary   add coreVector2                  --inline-children --omit-names")
    debugger.HandleCommand("type summary   add coreVector3                  --inline-children --omit-names")
    debugger.HandleCommand("type summary   add coreVector4                  --inline-children --omit-names")
    debugger.HandleCommand("type summary   add coreFlow                     --inline-children --omit-names")
    debugger.HandleCommand("type summary   add coreString                   --inline-children --omit-names")
    debugger.HandleCommand("type summary   add coreHashString               --inline-children --omit-names")


# command script import <path>/core-engine/projects/cmake/visualizer.py