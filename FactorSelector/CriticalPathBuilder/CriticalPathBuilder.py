from csv import reader
from collections import deque
from RequestTracker import RequestTracker
from SynchronizationObjectAggregator import SynchronizationObjectAggregator

# TODO need to add support for try_locks
# TODO need to add support for state transitions like the following for thread1
# executing semanticID1 -> executing semanticID2 -> executing semanticID1
class CriticalPathBuilder:
    def __init__(self, filename):
        self.synchObjAgg = SynchronizationObjectAggregator()
        self.requestTracker = RequestTracker()
        self.blockedEdgeStack = deque()

        synchronizationLogReader = reader(filename)

        for row in synchronizationLogReader:
            self.requestTracker.AddLogRow(row)
            self.synchObjAgg.AddLogRow(row)


    def __BuildHelper(self, segmentEndTime, currThreadID, timeSeries):
        opType, requestTime, objID = requestTracker.FindPrecedingRequest(currThreadID, segmentEndTime)

        unblockedSegment = -1
        i = 0
        for potentialBlockedEdge in self.blockedEdgeStack:
            if lastBlockingRequest.endTime <= potentialBlockedEdge[0]:
                unblockedSegment = i

            i += 1

        leftTimeBound = None
        nextThreadID = None
        # We have a blocked-by edge
        if unblockedSegment != -1:
            leftTimeBound = self.blockedEdgeStack[unblockedSegment][0]
            timeSeries.appendleft((currThreadID, leftTimeBound, segmentEndTime))

            nextThreadID = self.blockedEdgeStack[unblockedSegment][1]

            # We've finished the critical path for the semantic interval
            if nextThreadID == -1:
                return timeSeries

            for j in range(unblockedSegment + 1):
                self.blockedEdgeStack.pop_front()

        # We're blocked by some other thread
        else:
            self.blockedEdgeStack.appendleft((lastBlockingRequest.startTime, currThreadID))
            timeSeries.appendleft((currThreadID, lastBlockingRequest.endTime, segmentEndTime))

            leftTimeBound, nextThreadID = self.synchObjAgg.GetDependenceEdge(requestTime, objID, opType)

        return self.__BuildHelper(leftTimeBound, nextThreadID, timeSeries)


    # Returns a list of tuples, (threadID, startTime, endTime) where each tuple
    # represents a single segment of the critical path.  Note that if a tuple
    # represents an event creation wait-for relationship (the time between when
    # an event is created and picked up), its threadID will be -1.
    def Build(self, semIntStartTime, semIntEndTime, endingThreadID):
        # Take threadID of -1 to indicate that whichever thread is executing
        # at the end of the semantic interval is the final thread in the
        # critical path's time series.
        self.blockedEdgeStack.append((semIntStartTime, -1))

        timeSeries = self.__BuildHelper(semIntStartTime, semIntEndTime, endingThreadID, deque())

        self.blockedEdgeStack.clear()
        return timeSeries
