from locust import HttpUser, between, task

class simple_user(HttpUser):
    # wait_time = between(1, 2)
    host = "http://localhost:1234"
    network_timeout = 10

    @task
    def output_cgi_task(self):
        self.client.get("/output.cgi", timeout=10)
