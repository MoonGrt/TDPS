import uasyncio as asyncio

async def task1():
    while True:
        print("Task 1")
        await asyncio.sleep(1)

async def task2():
    while True:
        print("Task 2")
        await asyncio.sleep(2)

loop = asyncio.get_event_loop()
loop.create_task(task1())
loop.create_task(task2())
loop.run_forever()
