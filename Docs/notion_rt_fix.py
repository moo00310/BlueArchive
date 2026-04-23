# -*- coding: utf-8 -*-
import requests, json

import os
TOKEN = os.environ.get("NOTION_TOKEN", "")
PARENT_PAGE_ID = "34573507-9da8-8159-8209-c7a01de1cfa8"

HEADERS = {
    "Authorization": f"Bearer {TOKEN}",
    "Content-Type": "application/json",
    "Notion-Version": "2022-06-28",
}

def p(text):
    return {"object":"block","type":"paragraph","paragraph":{"rich_text":[{"type":"text","text":{"content":text}}]}}

def p_rich(parts):
    # parts: list of (text, bold, code, color)
    rt = []
    for item in parts:
        if isinstance(item, str):
            rt.append({"type":"text","text":{"content":item}})
        else:
            text, bold, is_code, color = item
            ann = {}
            if bold: ann["bold"] = True
            if is_code: ann["code"] = True
            if color: ann["color"] = color
            rt.append({"type":"text","text":{"content":text},"annotations":ann})
    return {"object":"block","type":"paragraph","paragraph":{"rich_text":rt}}

def h1(text):
    return {"object":"block","type":"heading_1","heading_1":{"rich_text":[{"type":"text","text":{"content":text}}]}}

def h2(text):
    return {"object":"block","type":"heading_2","heading_2":{"rich_text":[{"type":"text","text":{"content":text}}]}}

def h3(text):
    return {"object":"block","type":"heading_3","heading_3":{"rich_text":[{"type":"text","text":{"content":text}}]}}

def bullet(text, bold_prefix=None):
    if bold_prefix:
        rt = [{"type":"text","text":{"content":bold_prefix},"annotations":{"bold":True}},
              {"type":"text","text":{"content":text}}]
    else:
        rt = [{"type":"text","text":{"content":text}}]
    return {"object":"block","type":"bulleted_list_item","bulleted_list_item":{"rich_text":rt}}

def code(content, lang="c++"):
    return {"object":"block","type":"code","code":{"rich_text":[{"type":"text","text":{"content":content}}],"language":lang}}

def divider():
    return {"object":"block","type":"divider","divider":{}}

def callout(text, emoji="💡"):
    return {"object":"block","type":"callout","callout":{"rich_text":[{"type":"text","text":{"content":text}}],"icon":{"type":"emoji","emoji":emoji}}}

def quote(text):
    return {"object":"block","type":"quote","quote":{"rich_text":[{"type":"text","text":{"content":text}}]}}

def create_page(parent_id, title, children):
    data = {
        "parent": {"type":"page_id","page_id": parent_id},
        "properties": {"title": {"title": [{"type":"text","text":{"content":title}}]}},
        "children": children[:100]
    }
    r = requests.post("https://api.notion.com/v1/pages", headers=HEADERS, data=json.dumps(data))
    r.raise_for_status()
    page_id = r.json()["id"]
    # append remaining blocks if > 100
    remaining = children[100:]
    while remaining:
        append_blocks(page_id, remaining[:100])
        remaining = remaining[100:]
    return page_id

def append_blocks(block_id, children):
    data = {"children": children}
    r = requests.patch(f"https://api.notion.com/v1/blocks/{block_id}/children", headers=HEADERS, data=json.dumps(data))
    r.raise_for_status()
    return r.json()

# ============================================================
# 페이지 내용 구성
# ============================================================

blocks = [
    callout("RenderTarget 기반 3D 캐릭터 프리뷰 시스템에서 발생한 검은 테두리(Black Fringing) + 엣지 shimmer(이글거림) 문제를 분석하고 해결한 기록입니다.", "📋"),
    divider(),

    # ── 1. 시스템 구조 ──
    h1("1. 시스템 구조"),
    p("캐릭터 선택 UI에 실시간 3D 캐릭터 프리뷰를 구현하기 위해 SceneCaptureComponent2D 두 개와 RenderTarget 두 개를 사용하는 이중 캡처 구조를 채택했습니다."),
    p(""),
    bullet("CaptureColor  →  ColorRT : SCS_FinalColorHDR, 캐릭터의 실제 색상 캡처"),
    bullet("CaptureMask   →  MaskRT  : SCS_FinalColorLDR + PP_StencilToAlpha 포스트프로세스로 실루엣 마스크 생성"),
    bullet("UI Material (M_UI_PreviewRT) 에서 두 RT를 합성해 배경 위에 캐릭터를 합성"),
    p(""),
    callout("왜 RT 하나로 안 했나? — UE5에서 SCS_FinalColorHDR/LDR은 항상 알파 채널에 1을 씁니다. 스텐실 버퍼에서 컬러 RT 알파 채널로 연결되는 경로가 파이프라인 구조상 존재하지 않아, 단일 RT 알파 방식은 동작하지 않습니다.", "⚠️"),
    divider(),

    # ── 2. 문제 1: 검은 테두리 ──
    h1("2. 문제 1 — 검은 테두리 (Black Fringing)"),
    h2("증상"),
    p("캐릭터 외곽선에 검은색 테두리가 나타남. 특히 밝은 배경 위에서 두드러짐."),
    p(""),
    h2("원인 분석"),
    p("문제는 UI Material의 블렌딩 방식과 Premultiplied Alpha의 충돌이었습니다."),
    p(""),
    p("ColorRT 엣지 픽셀: 캐릭터 색상 × mask (배경은 검은색 = 0으로 곱해짐)"),
    p("기존 Material에서 Multiply 노드 사용: output = Lerp(bg, ColorRT, mask) = bg×(1−mask) + (char_color×mask)×mask"),
    p(""),
    callout("결과: 엣지에서 mask²이 적용되어 double-darkening 발생 → 검은 테두리", "❌"),
    p(""),
    h2("해결"),
    p("Premultiplied Alpha 합성 공식:   output = bg × (1−Opacity) + FinalColor"),
    p("이 공식은 FinalColor가 이미 alpha로 곱해진(premultiplied) 값임을 전제합니다."),
    p("ColorRT 자체가 이미 char_color × mask 형태이므로 추가 곱셈 없이 그대로 Final Color에 연결하면 됩니다."),
    p(""),
    bullet("Multiply 노드 제거"),
    bullet("ColorTex.RGB → Final Color 직결"),
    bullet("MaskTex.R → Opacity"),
    bullet("Blend Mode: Translucent → AlphaComposite (Premultiplied Alpha) 변경"),
    p(""),
    quote("AlphaComposite 모드는 UE5 Material 에서 Blend Mode 드롭다운 → Translucent 아래 항목으로 존재합니다."),
    divider(),

    # ── 3. 문제 2: Shimmer ──
    h1("3. 문제 2 — 엣지 Shimmer (이글거림 / 깜빡임)"),
    h2("증상"),
    p("캐릭터 외곽 픽셀이 프레임마다 미세하게 어긋나며 shimmer(이글거림)가 발생. 특히 정지한 캐릭터에서도 엣지가 떨림."),
    p(""),
    h2("원인 분석"),
    p("UE5의 TSR (Temporal Super Resolution) / TAA는 서브픽셀 재투영 정확도를 높이기 위해 매 프레임 렌더링 위치에 서브픽셀 지터(jitter)를 적용합니다."),
    p(""),
    bullet("CaptureColor와 CaptureMask가 서로 다른 AA 상태로 캡처되면, 프레임마다 엣지 픽셀 위치가 미세하게 어긋남"),
    bullet("SetPostProcessing(false)로도 TSR/TAA 지터가 완전히 꺼지지 않음"),
    bullet("결과: 두 RT의 엣지가 프레임마다 다르게 캡처 → UI 합성 시 shimmer 발생"),
    p(""),
    h2("해결"),
    p("두 SceneCaptureComponent2D 모두에서 ShowFlags를 통해 AA / TAA / MotionBlur를 명시적으로 비활성화합니다."),
    p(""),
    code(
        "// ===== Color Capture =====\n"
        "CaptureColor->ShowFlags = FEngineShowFlags(EShowFlagInitMode::ESFIM_Game);\n"
        "CaptureColor->ShowFlags.SetPostProcessing(false);\n"
        "// TSR/TAA가 프레임마다 서브픽셀 지터를 걸어 엣지가 이글거리는 원인.\n"
        "// SetPostProcessing(false)로도 완전히 꺼지지 않으므로 명시적으로 비활성화.\n"
        "CaptureColor->ShowFlags.SetAntiAliasing(false);\n"
        "CaptureColor->ShowFlags.SetTemporalAA(false);\n"
        "CaptureColor->ShowFlags.SetMotionBlur(false);\n\n"
        "// 노출 고정 (자동 노출로 인한 밝기 변동 방지)\n"
        "CaptureColor->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;\n"
        "CaptureColor->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;\n"
        "CaptureColor->PostProcessSettings.AutoExposureMinBrightness = 1.0f;\n"
        "CaptureColor->PostProcessSettings.AutoExposureMaxBrightness = 1.0f;\n\n"
        "// ===== Mask Capture =====\n"
        "CaptureMask->ShowFlags = FEngineShowFlags(EShowFlagInitMode::ESFIM_Game);\n"
        "CaptureMask->ShowFlags.SetPostProcessing(true); // PP는 켜야 StencilToAlpha 작동\n"
        "// Color와 Mask 두 캡처의 AA 상태가 다르면 엣지가 프레임마다 어긋나 shimmer 발생\n"
        "CaptureMask->ShowFlags.SetAntiAliasing(false);\n"
        "CaptureMask->ShowFlags.SetTemporalAA(false);\n"
        "CaptureMask->ShowFlags.SetMotionBlur(false);",
        "c++"
    ),
    p(""),
    callout("주의: UE 5.7 기준 FPostProcessSettings 에는 AntiAliasingMethod 필드가 없습니다. PostProcessSettings로 AA를 끄려 하면 컴파일 오류가 납니다. ShowFlags 만 사용해야 합니다.", "⚠️"),
    divider(),

    # ── 4. 정리 ──
    h1("4. 수정 요약"),
    p(""),
    h3("C++ (BAPreviewCharacter.cpp — Init 함수)"),
    bullet("CaptureColor / CaptureMask 양쪽에 ShowFlags로 AA, TAA, MotionBlur 비활성화"),
    bullet("CaptureColor에 AutoExposure Min/Max = 1.0 고정"),
    p(""),
    h3("UI Material (M_UI_PreviewRT)"),
    bullet("Multiply 노드 제거 → ColorTex.RGB를 Final Color에 직결"),
    bullet("MaskTex.R → Opacity"),
    bullet("Blend Mode: AlphaComposite (Premultiplied Alpha)"),
    p(""),
    callout("결과: 검은 테두리 제거 + 엣지 shimmer 완전 제거. 정지 상태에서도 안정적인 렌더링 확인.", "✅"),
]

print("Creating RenderTarget Fix page...")
page_id = create_page(PARENT_PAGE_ID, "RenderTarget 3D 프리뷰 — 검은 테두리 & Shimmer 수정", blocks)
print(f"Done! Page ID: {page_id}")
print(f"URL: https://notion.so/{page_id.replace('-','')}")
