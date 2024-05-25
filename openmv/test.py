

# # 从头部放入数据
# for i in range(5):
#     q.appendleft(i)  # 从头部添加元素

# print("Queue:", q)

# # 从尾部取出数据
# while q:
#     item = q.pop()  # 从尾部取出元素
#     print(f"Item: {item}, Remaining Queue: {q}")


# from collections import deque

# # 创建一个双向队列
# q = deque()

# def delay(data):
#     global q
#     q.append(data)
#     if len(q) > 5:
#         return q.popleft()

# print(delay(1))
# print(delay(2))
# print(delay(3))
# print(delay(4))
# print(delay(5))
# print(delay(6))
# print(delay(7))
# print(delay(8))
# print(delay(9))



# 创建一个空列表，模拟双向队列
q = []

# 定义一个函数，用于往队列中放入数据并实现队列大小限制
def delay(data, max_size=3):
    global q
    q.append(data)
    if len(q) > max_size:
        return q.pop(0)  # 从队列头部取出数据

print(delay(1))
print(delay(2))
print(delay(3))
print(delay(4))
print(delay(5))
print(delay(6))
print(delay(7))
print(delay(8))
print(delay(9))
