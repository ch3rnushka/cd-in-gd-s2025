#include "rasterizer_renderer.h"

#include "utils/resource_utils.h"


void cg::renderer::rasterization_renderer::init()
{
	rasterizer = std::make_shared<cg::renderer::rasterizer<cg::vertex, cg::unsigned_color>>();
	rasterizer->set_viewport(
			settings->width, settings->height);
	render_target = std::make_shared<cg::resource<cg::unsigned_color>>(
			settings->width, settings->height);
	depth_buffer = std::make_shared<cg::resource<float>>(
			settings->width, settings->height);
	rasterizer->set_render_target(render_target, depth_buffer);

	model = std::make_shared<cg::world::model>();
	model->load_obj(settings->model_path);

	camera = std::make_shared<cg::world::camera>();
	camera->set_height(static_cast<float>(settings->height));
	camera->set_width(static_cast<float>(settings->width));
	camera->set_position(float3{
			settings->camera_position[0],
			settings->camera_position[1],
			settings->camera_position[2]
	});
	camera->set_phi(settings->camera_phi);
	camera->set_theta(settings->camera_theta);
	camera->set_angle_of_view(settings->camera_angle_of_view);
	camera->set_z_near(settings->camera_z_near);
	camera->set_z_far(settings->camera_z_far);
}
void cg::renderer::rasterization_renderer::render()
{
	auto start = std::chrono::high_resolution_clock::now();

	rasterizer->clear_render_target({255, 255, 255});

	float4x4 matrix = mul(
		camera->get_projection_matrix(),
		camera->get_view_matrix(),
		model->get_world_matrix()
		);
	rasterizer->vertex_shader = [&](float4 vertex, cg::vertex vertex_data) {
		auto processed = mul(matrix, vertex);
		return std::make_pair(processed, vertex_data);
	};
	
	rasterizer->pixel_shader = [](cg::vertex vertex_data, float x) {
		cg::color final_color;
		float3 ambient = vertex_data.ambient;
		float3 diffuse = vertex_data.diffuse;
		float3 result_color = ambient + diffuse;
		result_color = clamp(result_color, 0.f, 1.f);
		return cg::color::from_float3(result_color);
	};
	
	for (size_t shape_id = 0; shape_id < model->get_index_buffers().size(); shape_id++){
		rasterizer->set_vertex_buffer(model->get_vertex_buffers()[shape_id]);
		rasterizer->set_index_buffer(model->get_index_buffers()[shape_id]);
		rasterizer->draw(
			model->get_index_buffers()[shape_id]->count(), 0
		);
	}
	
	auto stop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float, std::milli> dutation = stop-start;
	std::cout << "Clearing took " << dutation.count() << "ms\n";
	
	cg::utils::save_resource(*render_target, settings->result_path);
}

void cg::renderer::rasterization_renderer::destroy() {}

void cg::renderer::rasterization_renderer::update() {}