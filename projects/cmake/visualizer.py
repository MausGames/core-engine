#!/usr/bin/python
import lldb


def coreResourcePtrVisualizer(value, internal_dict):

    pCursor  = value.GetChildMemberWithName("m_pHandle")
    iPointer = pCursor.GetValueAsUnsigned(0)

    if iPointer == 0:
        return "(null)"

    pCursor  = pCursor.GetValueForExpressionPath(".m_sName._M_dataplus._M_p")
    sSummary = pCursor.GetSummary()

    if sSummary == "\"\"":
        return "(custom)"

    return F"({sSummary})"


def __lldb_init_module(debugger, internal_dict):

    debugger.HandleCommand("type summary add --python-function " + __name__ + ".coreResourcePtrVisualizer -x \"coreResourcePtr<.+>\"")
    debugger.HandleCommand("type summary add --inline-children --omit-names coreVector2")
    debugger.HandleCommand("type summary add --inline-children --omit-names coreVector3")
    debugger.HandleCommand("type summary add --inline-children --omit-names coreVector4")
    debugger.HandleCommand("type summary add --inline-children --omit-names coreFlow")


# command script import <path>/CoreEngine/projects/cmake/visualizer.py