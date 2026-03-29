if (ImGui::Begin(imgui_id.c_str())) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Get the current ImGui cursor position
  ImVec2 canvas_p0   = ImGui::GetCursorScreenPos();    // ImDrawList API uses screen coordinates!
  ImVec2 canvas_size = ImGui::GetContentRegionAvail(); // Resize canvas to what's available
  
  // guarantee a minimum canvas size
  canvas_size.x = std::max(canvas_size.x, 256.0f);
  canvas_size.y = std::max(canvas_size.y, 250.0f);
  
  ImVec2 canvas_p1 = ImVec2{canvas_p0.x + canvas_size.x, canvas_p0.y + canvas_size.y};
  
  ImGui::InvisibleButton("##canvas", canvas_size,
				  ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight |
								  ImGuiButtonFlags_MouseButtonMiddle);
  
  const bool canvas_hovered = ImGui::IsItemHovered(); // Hovered
  const bool canvas_active  = ImGui::IsItemActive();  // Held
  
  // const ImVec2 canvas_p0 = ImGui::GetItemRectMin(); // alternatively we can get the rectangle like this
  // const ImVec2 canvas_p1 = ImGui::GetItemRectMax();
  
  // Draw border and background color
  ImGuiIO& io = ImGui::GetIO();
  draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
  draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
  
  // TODO: shift or ctrl to slow zoom movement
  // if (canvas_active) {
  float zoom_rate   = 0.1f;
  float zoom_mouse  = io.MouseWheel * zoom_rate; //-0.1f 0.0f 0.1f
  float hzoom_mouse = zoom_mouse * 0.5f;
  float zoom_delta  = zoom_mouse *
							  open_images[i].transform.scale.x; // each step grows or shrinks image by 10%
  
  ImVec2 old_scale = open_images[i].transform.scale;
  // on screen (top left of image)
  ImVec2 old_origin = {canvas_p0.x + open_images[i].transform.translate.x,
				  canvas_p0.y + open_images[i].transform.translate.y};
  // on screen (bottom right of image)
  ImVec2 old_p1 = {old_origin.x + (open_images[i].width * open_images[i].transform.scale.x),
				  old_origin.y + (open_images[i].height * open_images[i].transform.scale.y)};
  // on screen (center of what we get to see), when adjusting scale this doesn't change!
  ImVec2 old_and_new_canvas_center = {
				  canvas_p0.x + canvas_size.x * 0.5f, canvas_p0.y + canvas_size.y * 0.5f};
  // in image coordinate offset of the center
  ImVec2 image_center = {
				  old_and_new_canvas_center.x - old_origin.x, old_and_new_canvas_center.y - old_origin.y};
  
  ImVec2 old_uv_image_center = {
				  image_center.x / (open_images[i].width * open_images[i].transform.scale.x),
				  image_center.y / (open_images[i].height * open_images[i].transform.scale.y)};
  
  open_images[i].transform.scale.x += zoom_delta;
  open_images[i].transform.scale.y += zoom_delta;
  
  // 2.0f -> 2x zoom in
  // 1.0f -> normal
  // 0.5f -> 2x zoom out
  // TODO: clamp based on image size, do we go pixel level?
  open_images[i].transform.scale.x = std::clamp(open_images[i].transform.scale.x, 0.01f, 100.0f);
  open_images[i].transform.scale.y = std::clamp(open_images[i].transform.scale.y, 0.01f, 100.0f);
  
  // on screen new target center
  ImVec2 new_image_center = {(open_images[i].width * open_images[i].transform.scale.x *
																  old_uv_image_center.x),
				  (open_images[i].height * open_images[i].transform.scale.y *
														  old_uv_image_center.y)};
  
  // readjust to center
  open_images[i].transform.translate.x -= new_image_center.x - image_center.x;
  open_images[i].transform.translate.y -= new_image_center.y - image_center.y;
  
  // 0 out second parameter if a context menu is open
  if (canvas_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 1.0f)) {
	  open_images[i].transform.translate.x += ImGui::GetIO().MouseDelta.x;
	  open_images[i].transform.translate.y += ImGui::GetIO().MouseDelta.y;
  }
  
  const ImVec2 origin(canvas_p0.x + open_images[i].transform.translate.x,
				  canvas_p0.y + open_images[i].transform.translate.y); // Lock scrolled origin
  
  // we need to control the rectangle we're going to draw and the uv coordinates
  const ImVec2 image_p1 = {origin.x + (open_images[i].transform.scale.x * open_images[i].width),
				  origin.y + (open_images[i].transform.scale.x * open_images[i].height)};
  
  const ImVec2 mouse_pos_in_canvas(imio.MousePos.x - origin.x, imio.MousePos.y - origin.y);
  
  draw_list->PushClipRect(ImVec2{canvas_p0.x + 2.0f, canvas_p0.y + 2.0f},
				  ImVec2{canvas_p1.x - 2.0f, canvas_p1.y - 2.0f}, true);
  // draw things
  draw_list->AddImage(open_images[i].texture_id, origin, image_p1);
  // draw things
  draw_list->PopClipRect();
}
ImGui::End();