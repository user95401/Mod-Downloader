#include <_main.hpp>
#include <_updater.hpp>

class ModsLayer : public CCNode {};

#include <Geode/modify/CCLayer.hpp>
class $modify(ModsLayerExt, CCLayer) {
	bool init() {
		if (!CCLayer::init()) return false;

		if (typeinfo_cast<ModsLayer*>(this)) queueInMainThread(
			[this] {
				if (auto search_input = typeinfo_cast<TextInput*>(getChildByIDRecursive("search-input"))) {
					search_input->setCommonFilter(CommonFilter::Any);
					auto org_callback = public_cast(search_input, m_onInput);
					search_input->setCallback(
						[org_callback](std::string str) {
							auto is_url = string::startsWith(str, "https://") or string::startsWith(str, "http://");
							if (is_url) {
								auto file = fs::path(str).filename();
								auto file_split = string::split(file.string(), "^");
								auto path = dirs::getModsDir() / file_split[file_split.size() - 1];
								str = string::split(str, "^")[0];
								if (!CCScene::get()->getChildByIDRecursive("download-dialog"_spr)) {
									MDPopup* pop = MDPopup::create(
										"Download This?",
										"### Download file at" "\n"
										+ str + "\n"
										"#### in to mods folder?" "\n"
										"`" + path.string() + "`"
										, "Download", nullptr,
										[str, path](bool) {
											auto req = web::WebRequest();
											auto listener = new EventListener<web::WebTask>;
											Ref<Notification> notify = Notification::create(
												"Downloading...",
												CCSprite::create((
													getMod()->getTempDir().string() + "/logo.png"
													).data()),
												0.f
											);
											Ref<Slider> loadingBar = Slider::create(nullptr, nullptr);
											notify->addChild(loadingBar);
											loadingBar->m_touchLogic->m_thumb->setVisible(0);
											loadingBar->setPositionY(-26.f);
											listener->bind(
												[str, path, loadingBar, notify](web::WebTask::Event* e) {
													if (web::WebProgress* prog = e->getProgress()) {
														if (loadingBar) loadingBar->setValue(prog->downloadProgress().value_or(0.f) / 100);
													}
													if (web::WebResponse* res = e->getValue()) {
														std::string data = res->string().unwrapOr("no res");
														if (res->code() < 399) {
															res->into(path);
															notify->hide();
															notify->setVisible(0);

															auto end_notify = Notification::create(
																"Downloaded!",
																CCSprite::create((
																	getMod()->getTempDir().string() + "/logo.png"
																	).data()),
																3.f
															);
															end_notify->show();
														}
														else {
															auto asd = geode::createQuickPopup(
																"Request exception",
																data,
																"Nah", nullptr, 420.f, nullptr, false
															);
															asd->show();
															notify->hide();
														};
													}
												}
											);
											notify->show();
											listener->setFilter(req.send("GET", str));
										}
									);
									pop->setID("download-dialog"_spr);
									pop->show();
								};
							}
							org_callback(str);
						}
					);
				}
			}
		);

		return true;
	}
};
