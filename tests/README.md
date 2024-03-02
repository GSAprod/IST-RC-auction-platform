## Run the tests

These were some of the tests used to make sure the server and the client are behaving properly.

To run these tests, compile the project and run a server using the instructions
supplied on the README file.

Then, on a second terminal, navigate to the `client` directory and run the following command:

```
./user [-n SERVER_IP] [-p SERVER_PORT] < ../tests/script_XX.txt
```

where `XX` is the number of the test you want to run.