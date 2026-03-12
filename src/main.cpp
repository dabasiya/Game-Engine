#include <Application.h>
#include <Instrumentor.h>


int main() {
	HZ_PROFILE_BEGIN_SESSION("GameEngine", "test.json");
	auto& app = Application::GetInstance();

	app.Run();

	app.Release();
	HZ_PROFILE_END_SESSION();
}


