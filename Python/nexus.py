import csv

ProducersTableEntryListSize = 10
ConsumersTableEntryListSize = 10

class Task:
    def __init__(self, id, num_inputs, num_outputs, inputs, outputs):
        self.id = id
        self.num_inputs = num_inputs
        self.num_outputs = num_outputs
        self.inputs = inputs
        self.outputs = outputs
    def __str__(self):
        return "<Task {}: ({}, {}, {}, {})".format(self.id, self.num_inputs, self.num_outputs, self.inputs, self.outputs)

class TaskTableEntry:
    def __init__(self, task):
        self.task = task
        self.deps = 0
    def __str__(self):
        st = "<Entry {} ({})>".format(self.deps, self.task)
        return st

class TaskTable:
    def __init__(self, size):
        self.size = size
        self.tasks = []
    def add(self, task):
        if isinstance(task, Task):
            if len(self.tasks) < self.size:
                self.tasks.append(TaskTableEntry(task))
                return self.getTask(task.id)
    def dump(self):
        for task in self.tasks:
            print(task)
    def getTask(self, id):
        for task in self.tasks:
            if task.task.id == id:
                return task
    def __getitem__(self, index):
        if index >= 0 and index < len(self.tasks):
            return self.tasks[index]
    def __iter__(self):
        self.index = 0
        return self
    def __next__(self):
        if self.index < len(self.tasks):
            self.index += 1
            return self.tasks[self.index-1]
        raise StopIteration
    def removeTask(self, id):
        task = self.getTask(id)
        self.tasks.remove(task)

class KickOfList:
    def __init__(self, size):
        self.size = size
        self.tasks = []
    def add(self, task):
        if isinstance(task, Task):
            if len(self.tasks) < self.size:
                self.tasks.append(task)
                return True
        return False
    def pop(self):
        if len(self.tasks) > 0:
            return self.tasks.pop()
    def __str__(self):
        st = "<Kick Of List: ("
        for task in self.tasks:
            st += str(task) + ","
        st += ")"
        return st
    def __getitem__(self, index):
        if index >= 0 and index < len(self.tasks):
            return self.tasks[index]
    def remove(self, id):
        if self.tasks[0].id == id:
            del self.tasks[0]

class ProducersTableEntry:
    def __init__(self, addr, task):
        self.addr = addr
        self.list = KickOfList(ProducersTableEntryListSize)
        self.list.add(task)
    def add(self, task):
        if isinstance(task, Task):
            return self.list.add(task)
    def __str__(self):
        return "<Producers Entry {}:({})".format(self.addr, self.list)

class ProducersTable:
    def __init__(self, size):
        self.size = size
        self.entries = []
    def add(self, entry):
        if isinstance(entry, ProducersTableEntry):
            if len(self.entries) < self.size:
                self.entries.append(entry)
                return True
        return False

    def __getitem__(self, index):
        if index >=0 and index < len(self.entries):
            return self.entries[index]
    def dump(self):
        for entry in self.entries:
            print(entry)
    def getEntry(self, address):
        for entry in self.entries:
            if entry.addr == address:
                return entry
    def removeTask(self, id):
        for entry in self.entries:
            entry.list.remove(id)

class ConsumersTableEntry:
    def __init__(self, addr):
        self.addr = addr
        self.list = KickOfList(ConsumersTableEntryListSize)
        self.deps = 1
    def add(self, task):
        if isinstance(task, Task):
            #self.deps += 1
            return self.list.add(task)

    def incDeps(self):
        self.deps += 1
    def __str__(self):
        return "<Consumers Entry {}:({}, {})".format(self.addr, self.deps, self.list)

class ConsumersTable:
    def __init__(self, size):
        self.size = size
        self.entries = []
    def add(self, entry):
        if isinstance(entry, ConsumersTableEntry):
            if len(self.entries) < self.size:
                self.entries.append(entry)
                return True
        return False

    def __getitem__(self, index):
        if index >=0 and index < len(self.entries):
            return self.entries[index]
    def dump(self):
        for entry in self.entries:
            print(entry)
    def getEntry(self, address):
        for entry in self.entries:
            if entry.addr == address:
                return entry
    def removeTask(self, id):
        for entry in self.entries:
            if entry.deps == 0 and entry.list[0].id == id:
                entry.list.remove(id)

class Nexus:
    def __init__(self):
        self.taskTable = TaskTable(10)
        self.prodTable = ProducersTable(10)
        self.consTable = ConsumersTable(10)
    def addInputProd(self, input, task):
        entry = self.prodTable.getEntry(input)
        if entry is not None:
            entry.add(task)
            return 1
        return 0
    def addInputCons(self, input, task):
        entry = self.prodTable.getEntry(input)
        if entry is None:
            entry = self.consTable.getEntry(input)
            if entry is not None:
                entry.incDeps()
            else:
                self.consTable.add(ConsumersTableEntry(input))
        else:
            entry = self.consTable.getEntry(input)
            if entry is not None:
                entry.add(task)
        return 0
    def addOutputProd(self, output, task):
        entry = self.prodTable.getEntry(output)
        if entry is not None:
            entry.add(task)
            return 1
        else:
            self.prodTable.add(ProducersTableEntry(output, task))
            return 0
    def addOutputCons(self, output, task):
        entry = self.consTable.getEntry(output)
        if entry is not None:
            entry.add(task)
            return 1
        return 0
    def addTask(self, task):
        if isinstance(task, Task):
            deps = 0
            taskEntry = self.taskTable.add(task)
            for input in task.inputs:
                deps += self.addInputProd(input, task)
                deps += self.addInputCons(input, task)
            for output in task.outputs:
                deps += self.addOutputCons(output, task)
                deps += self.addOutputProd(output, task)
            taskEntry.deps = deps
    def removeTask(self, id):
        self.taskTable.removeTask(id)
        self.prodTable.removeTask(id)
        self.consTable.removeTask(id)
    def dump(self):
        print("Dumping task table")
        self.taskTable.dump()
        print("Dumping producers table")
        self.prodTable.dump()
        print("Dumping consumers table")
        self.consTable.dump()


def main():
    taskTable = TaskTable(10)
    with open('nexus_tasks.csv') as f:
        reader = csv.reader(f)
        nexus = Nexus()
        for row in reader:
            id = int(row[0])
            num_inputs = int(row[2])
            num_outputs = int(row[3])
            inputs = row[4:num_inputs + 4]
            outputs = row[num_inputs + 4:]
            nexus.addTask(Task(id, num_inputs, num_outputs, inputs, outputs))
    #list = KickOfList(1)
    #prodTable = ProducersTable(10)
    #consTable = ConsumersTable(10)
    nexus.removeTask(6)
    nexus.dump()

if __name__ == '__main__':
    import os
    os.system("clear")
    main()
