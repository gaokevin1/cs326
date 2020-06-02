# Lab 11: Networking with Sockets

In this lab you'll get more familiar with using sockets to send messages between a client and server. Sample code has been provided for you.

## Main Takeaway
The point of this lab is to demonstrate how sockets work, as well as a fundamental aspect of their design: sockets transmit
simple streams of data, **not** discrete messages. As a result, `read()` must be called multiple times when reading from
a socket file descriptor to ensure all the data is received.


## Lab Steps:

1. Play around with the provided network code. Try to understand the workflow and function calls -- the man pages are a good way to understand how each function works.
2. Once you're comfortable with the code, follow STEP 1 in client.c. This makes the write() call send one byte at a time rather than the entire 128-byte message
3. Note that on the server side, you'll receive many more 'messages' that are broken up parts of the original message. They might actually be bigger than 1 byte, though...
4. Follow STEP 2, adding a sleep to the send loop. This forces each message to only contain a single byte.
5. Implement `read_len` in server.c. This function will read a specified number of bytes into a buffer, regardless of how many send/recv operations happen. Once `read_len` is complete, the server should only receive one message again (even if the client is sending 1 byte at a time)
6. Finally, we're sending 128-byte messages, which is wasteful: if a user only types 10 characters, we shouldn't send 128 bytes of data. Adapt the code to use a length-prefixed struct instead of a fixed-sized buffer. This means that if the user types 10 characters, you will only send 1 struct + 10 chars.

