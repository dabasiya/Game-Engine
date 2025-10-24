#include <Application.h>

int main() {

	auto& app = Application::GetInstance();

	app.Run();

	app.Release();
}


