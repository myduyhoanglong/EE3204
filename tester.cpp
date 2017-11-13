#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <chrono>
#include <iostream>

using namespace std;

int main() {
    int data_len, server_pid, client_pid;
    int port = 8888;
	int error = 0;
    for (data_len = 1000;  data_len <= 60000;  data_len += 1000) {
        for (int i = 0; i < 20; ++i) {
        	std::cout << "------------------------------------------" << std::endl;
        	std::cout << "Running with " << data_len << " - " << port<< std::endl;
            server_pid = fork();
            if (!server_pid) {
                execl("./tcp_ser4", "./tcp_ser4",
                        std::to_string(data_len).c_str(),
                        std::to_string(port).c_str(),
                        std::to_string(error).c_str(),
                        (char *)0);
                return 0;
            } else {
                client_pid = fork();
                if (!client_pid) {
                    // Child run client.
                    execl("./tcp_client4", "./tcp_client4", "localhost",
                            std::to_string(data_len).c_str(),
                            std::to_string(port).c_str(),
                            (char *)0);
                    return 0;
                } else {
                    // Parent wait for child to exit.
                    waitpid(client_pid, nullptr, 0);
                    waitpid(server_pid, nullptr, 0);
                }
            }
			port++;
			sleep(1);
        }
		//getchar();
    }
    return 0;
}
