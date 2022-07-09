.arm

.align 4

.section .text.getThreadCommandBuffer, "ax", %progbits
.global getThreadCommandBuffer
.type getThreadCommandBuffer, %function
getThreadCommandBuffer:
	mrc p15, 0, r0, c13, c0, 3
	add r0, #0x80
	bx lr

.section .text.svc_controlMemory, "ax", %progbits
.global svc_controlMemory
.type svc_controlMemory, %function
svc_controlMemory:
	stmfd sp!, {r0, r4}
	ldr r0, [sp, #0x8]
	ldr r4, [sp, #0x8+0x4]
	svc 0x01
	ldr r2, [sp], #4
	str r1, [r2]
	ldr r4, [sp], #4
	bx lr

.section .text.svc_queryMemory, "ax", %progbits
.global svc_queryMemory
.type svc_queryMemory, %function
svc_queryMemory:
	push {r0, r1, r4-r6}
	svc  0x02
	ldr  r6, [sp]
	str  r1, [r6]
	str  r2, [r6, #4]
	str  r3, [r6, #8]
	str  r4, [r6, #0xc]
	ldr  r6, [sp, #4]
	str  r5, [r6]
	add  sp, sp, #8
	pop  {r4-r6}
	bx   lr

.section .text.svc_exitProcess, "ax", %progbits
.global svc_exitProcess
.type svc_exitProcess, %function
svc_exitProcess:
	svc 0x03
	bx lr

.section .text.svc_createThread, "ax", %progbits
.global svc_createThread
.type svc_createThread, %function
svc_createThread:
	stmfd sp!, {r0, r4}
	ldr r0, [sp, #0x8]
	ldr r4, [sp, #0x8+0x4]
	svc 0x08
	ldr r2, [sp], #4
	str r1, [r2]
	ldr r4, [sp], #4
	bx lr

.section .text.svc_exitThread, "ax", %progbits
.global svc_exitThread
.type svc_exitThread, %function
svc_exitThread:
	svc 0x09
	bx lr

.section .text.svc_sleepThread, "ax", %progbits
.global svc_sleepThread
.type svc_sleepThread, %function
svc_sleepThread:
	svc 0x0A
	bx lr

.section .text.svc_createMutex, "ax", %progbits
.global svc_createMutex
.type svc_createMutex, %function
svc_createMutex:
	str r0, [sp, #-4]!
	svc 0x13
	ldr r3, [sp], #4
	str r1, [r3]
	bx lr

.section .text.svc_releaseMutex, "ax", %progbits
.global svc_releaseMutex
.type svc_releaseMutex, %function
svc_releaseMutex:
	svc 0x14
	bx lr

.section .text.svc_releaseSemaphore, "ax", %progbits
.global svc_releaseSemaphore
.type svc_releaseSemaphore, %function
svc_releaseSemaphore:
        str r0, [sp,#-4]!
        svc 0x16
        ldr r2, [sp], #4
        str r1, [r2]
        bx lr

.section .text.svc_createEvent, "ax", %progbits
.global svc_createEvent
.type svc_createEvent, %function
svc_createEvent:
	str r0, [sp,#-4]!
	svc 0x17
	ldr r2, [sp], #4
	str r1, [r2]
	bx lr

.section .text.svc_signalEvent, "ax", %progbits
.global svc_signalEvent
.type svc_signalEvent, %function
svc_signalEvent:
	svc 0x18
	bx lr

.section .text.svc_clearEvent, "ax", %progbits
.global svc_clearEvent
.type svc_clearEvent, %function
svc_clearEvent:
	svc 0x19
	bx lr

.section .text.svc_createMemoryBlock, "ax", %progbits
.global svc_createMemoryBlock
.type svc_createMemoryBlock, %function
svc_createMemoryBlock:
	str r0, [sp, #-4]!
	ldr r0, [sp, #4]
	svc 0x1E
	ldr r2, [sp], #4
	str r1, [r2]
	bx lr

.section .text.svc_mapMemoryBlock, "ax", %progbits
.global svc_mapMemoryBlock
.type svc_mapMemoryBlock, %function
svc_mapMemoryBlock:
	svc 0x1F
	bx lr

.section .text.svc_unmapMemoryBlock, "ax", %progbits
.global svc_unmapMemoryBlock
.type svc_unmapMemoryBlock, %function
svc_unmapMemoryBlock:
	svc 0x20
	bx lr

.section .text.svc_arbitrateAddress, "ax", %progbits
.global svc_arbitrateAddress
.type svc_arbitrateAddress, %function
svc_arbitrateAddress:
        svc 0x22
        bx lr

.section .text.svc_closeHandle, "ax", %progbits
.global svc_closeHandle
.type svc_closeHandle, %function
svc_closeHandle:
	svc 0x23
	bx lr

.section .text.svc_waitSynchronization1, "ax", %progbits
.global svc_waitSynchronization1
.type svc_waitSynchronization1, %function
svc_waitSynchronization1:
	svc 0x24
	bx lr

.section .text.svc_waitSynchronizationN, "ax", %progbits
.global svc_waitSynchronizationN
.type svc_waitSynchronizationN, %function
svc_waitSynchronizationN:
	str r5, [sp, #-4]!
	mov r5, r0
	ldr r0, [sp, #0x4]
	ldr r4, [sp, #0x4+0x4]
	svc 0x25
	str r1, [r5]
	ldr r5, [sp], #4
	bx lr

.section .text.svc_getSystemTick, "ax", %progbits
.global svc_getSystemTick
.type svc_getSystemTick, %function
svc_getSystemTick:
	svc 0x28
	bx lr

.section .text.svc_getSystemInfo, "ax", %progbits
.global svc_getSystemInfo
.type svc_getSystemInfo, %function
svc_getSystemInfo:
	stmfd sp!, {r0, r4}
	svc 0x2A
	ldr r4, [sp], #4
	str r1, [r4]
	str r2, [r4, #4]
	# str r3, [r4, #8] # ?
	ldr r4, [sp], #4
	bx lr

.section .text.svc_getProcessInfo, "ax", %progbits
.global svc_getProcessInfo
.type svc_getProcessInfo, %function
svc_getProcessInfo:
	stmfd sp!, {r0, r4}
	svc 0x2B
	ldr r4, [sp], #4
	str r1, [r4]
	str r2, [r4, #4]
	ldr r4, [sp], #4
	bx lr

.section .text.svc_connectToPort, "ax", %progbits
.global svc_connectToPort
.type svc_connectToPort, %function
svc_connectToPort:
	str r0, [sp,#-0x4]!
	svc 0x2D
	ldr r3, [sp], #4
	str r1, [r3]
	bx lr

.section .text.svc_sendSyncRequest, "ax", %progbits
.global svc_sendSyncRequest
.type svc_sendSyncRequest, %function
svc_sendSyncRequest:
	svc 0x32
	bx lr

.section .text.svc_getProcessId, "ax", %progbits
.global svc_getProcessId
.type svc_getProcessId, %function
svc_getProcessId:
	str r0, [sp,#-0x4]!
	svc 0x35
	ldr r3, [sp], #4
	str r1, [r3]
	bx lr
