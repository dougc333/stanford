
handle SIGLWP nostop noprint pass

define threads
p ListAllThreads()
end

document threads
The threads command will call the 107 thread library routine "ListAllThreads"
to print the current set of the live threads and their status.
end


define semaphores
p ListAllSemaphores()
end

document semaphores
The semaphores command will call the 107 thread library routine 
"ListAllSemaphores" to print the current set of the semaphores and 
their values.
end
set height 10000
