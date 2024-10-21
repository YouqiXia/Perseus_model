import unitlib

class ArchConfig:
    def __init__(self):
        pass
    
    def get_arch_name(self):
        return
    
class SimpleArchConfig():
    def __init__(self):
        self.fu_latency = {unitlib.func_type.ALU: 1,
                           unitlib.func_type.BRU: 1,
                           unitlib.func_type.CSR: 1,
                           unitlib.func_type.MUL: 1,
                           unitlib.func_type.DIV: 1,
                           unitlib.func_type.FPU: 1,
                           unitlib.func_type.LDU: 1,
                           unitlib.func_type.STU: 1}
        
        # do not modify by interface
        full_fu_array = [unitlib.func_type.ALU,
                         unitlib.func_type.BRU,
                         unitlib.func_type.CSR,
                         unitlib.func_type.MUL,
                         unitlib.func_type.DIV,
                         unitlib.func_type.FPU,
                         unitlib.func_type.LDU,
                         unitlib.func_type.STU]
        
        self.dispatch_map = [full_fu_array]
        
        # the number of pipeline after disptach stage
        self.dispatch_path_num = len(self.dispatch_map)

    def get_arch_name(self):
        return "simple_arch"

    def modify_fu_latency(self, fu_type, latency):
        self.fu_latency[fu_type] = latency

    def get_fu_latency(self):
        fu_latency_str = []
        for fu_name, latency in self.fu_latency.items():
            fu_latency_str.append(fu_name)
            fu_latency_str.append("|")
            fu_latency_str.append(f"{latency}")
            fu_latency_str.append("|")
        return fu_latency_str
    
    def get_dispatch_map(self):
        return self.dispatch_map
    
