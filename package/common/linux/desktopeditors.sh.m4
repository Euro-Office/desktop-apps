#!/bin/sh
include(defines.m4)dnl

copy_templates() {
  TEMPLATE_LANG="default"
  NEW_DOCX="_NEW_WORD"
  NEW_XLSX="_NEW_CELL"
  NEW_PPTX="_NEW_SLIDE"
  NEW_PDFF="_NEW_FORM"
  case $LANG in
    ar_* )
      TEMPLATE_LANG="ar-SA"
      NEW_DOCX="_NEW_WORD_ar"
      NEW_XLSX="_NEW_CELL_ar"
      NEW_PPTX="_NEW_SLIDE_ar"
      NEW_PDFF="_NEW_FORM_ar"
      ;;
    be_* )
      NEW_DOCX="_NEW_WORD_be"
      NEW_XLSX="_NEW_CELL_be"
      NEW_PPTX="_NEW_SLIDE_be"
      NEW_PDFF="_NEW_FORM_be"
      ;;
    bg_* )
      TEMPLATE_LANG="bg-BG"
      NEW_DOCX="_NEW_WORD_bg"
      NEW_XLSX="_NEW_CELL_bg"
      NEW_PPTX="_NEW_SLIDE_bg"
      NEW_PDFF="_NEW_FORM_bg"
      ;;
    ca_* )
      TEMPLATE_LANG="ca-ES"
      NEW_DOCX="_NEW_WORD_ca"
      NEW_XLSX="_NEW_CELL_ca"
      NEW_PPTX="_NEW_SLIDE_ca"
      NEW_PDFF="_NEW_FORM_ca"
      ;;
    cs_* )
      TEMPLATE_LANG="cs-CZ"
      NEW_DOCX="_NEW_WORD_cs"
      NEW_XLSX="_NEW_CELL_cs"
      NEW_PPTX="_NEW_SLIDE_cs"
      NEW_PDFF="_NEW_FORM_cs"
      ;;
    da_* )
      TEMPLATE_LANG="da-DK"
      NEW_DOCX="_NEW_WORD_da"
      NEW_XLSX="_NEW_CELL_da"
      NEW_PPTX="_NEW_SLIDE_da"
      NEW_PDFF="_NEW_FORM_da"
      ;;
    de_* )
      TEMPLATE_LANG="de-DE"
      NEW_DOCX="_NEW_WORD_de"
      NEW_XLSX="_NEW_CELL_de"
      NEW_PPTX="_NEW_SLIDE_de"
      NEW_PDFF="_NEW_FORM_de"
      ;;
    el_* )
      TEMPLATE_LANG="el-GR"
      NEW_DOCX="_NEW_WORD_el"
      NEW_XLSX="_NEW_CELL_el"
      NEW_PPTX="_NEW_SLIDE_el"
      NEW_PDFF="_NEW_FORM_el"
      ;;
    en_GB* )
      TEMPLATE_LANG="en-GB"
      ;;
    en_US* )
      TEMPLATE_LANG="en-US"
      ;;
    es_* )
      TEMPLATE_LANG="es-ES"
      NEW_DOCX="_NEW_WORD_es"
      NEW_XLSX="_NEW_CELL_es"
      NEW_PPTX="_NEW_SLIDE_es"
      NEW_PDFF="_NEW_FORM_es"
      ;;
    fi_* )
      TEMPLATE_LANG="fi-FI"
      NEW_DOCX="_NEW_WORD_fi"
      NEW_XLSX="_NEW_CELL_fi"
      NEW_PPTX="_NEW_SLIDE_fi"
      NEW_PDFF="_NEW_FORM_fi"
      ;;
    fr_* )
      TEMPLATE_LANG="fr-FR"
      NEW_DOCX="_NEW_WORD_fr"
      NEW_XLSX="_NEW_CELL_fr"
      NEW_PPTX="_NEW_SLIDE_fr"
      NEW_PDFF="_NEW_FORM_fr"
      ;;
    gl_* )
      TEMPLATE_LANG="gl-ES"
      NEW_DOCX="_NEW_WORD_gl"
      NEW_XLSX="_NEW_CELL_gl"
      NEW_PPTX="_NEW_SLIDE_gl"
      NEW_PDFF="_NEW_FORM_gl"
      ;;
    he_* )
      TEMPLATE_LANG="he-IL"
      NEW_DOCX="_NEW_WORD_he"
      NEW_XLSX="_NEW_CELL_he"
      NEW_PPTX="_NEW_SLIDE_he"
      NEW_PDFF="_NEW_FORM_he"
      ;;
    hu_* )
      TEMPLATE_LANG="hu-HU"
      NEW_DOCX="_NEW_WORD_hu"
      NEW_XLSX="_NEW_CELL_hu"
      NEW_PPTX="_NEW_SLIDE_hu"
      NEW_PDFF="_NEW_FORM_hu"
      ;;
    hy_* )
      TEMPLATE_LANG="hy-AM"
      NEW_DOCX="_NEW_WORD_hy"
      NEW_XLSX="_NEW_CELL_hy"
      NEW_PPTX="_NEW_SLIDE_hy"
      NEW_PDFF="_NEW_FORM_hy"
      ;;
    id_* )
      TEMPLATE_LANG="id-ID"
      NEW_DOCX="_NEW_WORD_id"
      NEW_XLSX="_NEW_CELL_id"
      NEW_PPTX="_NEW_SLIDE_id"
      NEW_PDFF="_NEW_FORM_id"
      ;;
    it_* )
      TEMPLATE_LANG="it-IT"
      NEW_DOCX="_NEW_WORD_it"
      NEW_XLSX="_NEW_CELL_it"
      NEW_PPTX="_NEW_SLIDE_it"
      NEW_PDFF="_NEW_FORM_it"
      ;;
    ja_* )
      TEMPLATE_LANG="ja-JP"
      NEW_DOCX="_NEW_WORD_ja"
      NEW_XLSX="_NEW_CELL_ja"
      NEW_PPTX="_NEW_SLIDE_ja"
      NEW_PDFF="_NEW_FORM_ja"
      ;;
    ko_* )
      TEMPLATE_LANG="ko-KR"
      NEW_DOCX="_NEW_WORD_ko"
      NEW_XLSX="_NEW_CELL_ko"
      NEW_PPTX="_NEW_SLIDE_ko"
      NEW_PDFF="_NEW_FORM_ko"
      ;;
    lo_* )
      NEW_DOCX="_NEW_WORD_lo"
      NEW_XLSX="_NEW_CELL_lo"
      NEW_PPTX="_NEW_SLIDE_lo"
      NEW_PDFF="_NEW_FORM_lo"
      ;;
    lv_* )
      TEMPLATE_LANG="lv-LV"
      NEW_DOCX="_NEW_WORD_lv"
      NEW_XLSX="_NEW_CELL_lv"
      NEW_PPTX="_NEW_SLIDE_lv"
      NEW_PDFF="_NEW_FORM_lv"
      ;;
    nb_* )
      TEMPLATE_LANG="nb-NO"
      NEW_DOCX="_NEW_WORD_nb"
      NEW_XLSX="_NEW_CELL_nb"
      NEW_PPTX="_NEW_SLIDE_nb"
      NEW_PDFF="_NEW_FORM_nb"
      ;;
    nl_* )
      TEMPLATE_LANG="nl-NL"
      NEW_DOCX="_NEW_WORD_nl"
      NEW_XLSX="_NEW_CELL_nl"
      NEW_PPTX="_NEW_SLIDE_nl"
      NEW_PDFF="_NEW_FORM_nl"
      ;;
    pl_* )
      TEMPLATE_LANG="pl-PL"
      NEW_DOCX="_NEW_WORD_pl"
      NEW_XLSX="_NEW_CELL_pl"
      NEW_PPTX="_NEW_SLIDE_pl"
      NEW_PDFF="_NEW_FORM_pl"
      ;;
    pt_BR* )
      TEMPLATE_LANG="pt-BR"
      NEW_DOCX="_NEW_WORD_ptbr"
      NEW_XLSX="_NEW_CELL_ptbr"
      NEW_PPTX="_NEW_SLIDE_ptbr"
      NEW_PDFF="_NEW_FORM_ptbr"
      ;;
    pt_* )
      TEMPLATE_LANG="pt-PT"
      NEW_DOCX="_NEW_WORD_pt"
      NEW_XLSX="_NEW_CELL_pt"
      NEW_PPTX="_NEW_SLIDE_pt"
      NEW_PDFF="_NEW_FORM_pt"
      ;;
    ro_* )
      TEMPLATE_LANG="ro-RO"
      NEW_DOCX="_NEW_WORD_ro"
      NEW_XLSX="_NEW_CELL_ro"
      NEW_PPTX="_NEW_SLIDE_ro"
      NEW_PDFF="_NEW_FORM_ro"
      ;;
    ru_* )
      TEMPLATE_LANG="ru-RU"
      NEW_DOCX="_NEW_WORD_ru"
      NEW_XLSX="_NEW_CELL_ru"
      NEW_PPTX="_NEW_SLIDE_ru"
      NEW_PDFF="_NEW_FORM_ru"
      ;;
    si_* )
      TEMPLATE_LANG="si-LK"
      NEW_DOCX="_NEW_WORD_si"
      NEW_XLSX="_NEW_CELL_si"
      NEW_PPTX="_NEW_SLIDE_si"
      NEW_PDFF="_NEW_FORM_si"
      ;;
    sk_* )
      TEMPLATE_LANG="sk-SK"
      NEW_DOCX="_NEW_WORD_sk"
      NEW_XLSX="_NEW_CELL_sk"
      NEW_PPTX="_NEW_SLIDE_sk"
      NEW_PDFF="_NEW_FORM_sk"
      ;;
    sl_* )
      TEMPLATE_LANG="sl-SI"
      NEW_DOCX="_NEW_WORD_sl"
      NEW_XLSX="_NEW_CELL_sl"
      NEW_PPTX="_NEW_SLIDE_sl"
      NEW_PDFF="_NEW_FORM_sl"
      ;;
    sq_* )
      TEMPLATE_LANG="sq-AL"
      NEW_DOCX="_NEW_WORD_sq"
      NEW_XLSX="_NEW_CELL_sq"
      NEW_PPTX="_NEW_SLIDE_sq"
      NEW_PDFF="_NEW_FORM_sq"
      ;;
    sr_RS@latin* )
      TEMPLATE_LANG="sr-Latn-RS"
      NEW_DOCX="_NEW_WORD_srlat"
      NEW_XLSX="_NEW_CELL_srlat"
      NEW_PPTX="_NEW_SLIDE_srlat"
      NEW_PDFF="_NEW_FORM_srlat"
      ;;
    sr_RS* )
      TEMPLATE_LANG="sr-Cyrl-RS"
      NEW_DOCX="_NEW_WORD_sr"
      NEW_XLSX="_NEW_CELL_sr"
      NEW_PPTX="_NEW_SLIDE_sr"
      NEW_PDFF="_NEW_FORM_sr"
      ;;
    sv_* )
      TEMPLATE_LANG="sv-SE"
      NEW_DOCX="_NEW_WORD_sv"
      NEW_XLSX="_NEW_CELL_sv"
      NEW_PPTX="_NEW_SLIDE_sv"
      NEW_PDFF="_NEW_FORM_sv"
      ;;
    tr_* )
      TEMPLATE_LANG="tr-TR"
      NEW_DOCX="_NEW_WORD_tr"
      NEW_XLSX="_NEW_CELL_tr"
      NEW_PPTX="_NEW_SLIDE_tr"
      NEW_PDFF="_NEW_FORM_tr"
      ;;
    uk_* )
      TEMPLATE_LANG="uk-UA"
      NEW_DOCX="_NEW_WORD_uk"
      NEW_XLSX="_NEW_CELL_uk"
      NEW_PPTX="_NEW_SLIDE_uk"
      NEW_PDFF="_NEW_FORM_uk"
      ;;
    ur_* )
      TEMPLATE_LANG="ur-PK"
      NEW_DOCX="_NEW_WORD_ur"
      NEW_XLSX="_NEW_CELL_ur"
      NEW_PPTX="_NEW_SLIDE_ur"
      NEW_PDFF="_NEW_FORM_ur"
      ;;
    vi_* )
      TEMPLATE_LANG="vi-VN"
      NEW_DOCX="_NEW_WORD_vi"
      NEW_XLSX="_NEW_CELL_vi"
      NEW_PPTX="_NEW_SLIDE_vi"
      NEW_PDFF="_NEW_FORM_vi"
      ;;
    zh_CN* )
      TEMPLATE_LANG="zh-CN"
      NEW_DOCX="_NEW_WORD_zhcn"
      NEW_XLSX="_NEW_CELL_zhcn"
      NEW_PPTX="_NEW_SLIDE_zhcn"
      NEW_PDFF="_NEW_FORM_zhcn"
      ;;
    zh_TW* )
      TEMPLATE_LANG="zh-TW"
      NEW_DOCX="_NEW_WORD_zhtw"
      NEW_XLSX="_NEW_CELL_zhtw"
      NEW_PPTX="_NEW_SLIDE_zhtw"
      NEW_PDFF="_NEW_FORM_zhtw"
      ;;
  esac
  SOURCE_DIR="/opt/M4_DESKTOPEDITORS_PREFIX/converter/empty/$TEMPLATE_LANG"

  if command -v xdg-user-dir &> /dev/null; then
    XDG_TEMPLATES_DIR=$(xdg-user-dir TEMPLATES)
  else
    echo "xdg-user-dir not installed"
    exit 1
  fi

  mkdir -pv "$XDG_TEMPLATES_DIR"
  cp -fv "$SOURCE_DIR/new.docx" "$XDG_TEMPLATES_DIR/$NEW_DOCX.docx"
  cp -fv "$SOURCE_DIR/new.xlsx" "$XDG_TEMPLATES_DIR/$NEW_XLSX.xlsx"
  cp -fv "$SOURCE_DIR/new.pptx" "$XDG_TEMPLATES_DIR/$NEW_PPTX.pptx"
  cp -fv "$SOURCE_DIR/new.pdf" "$XDG_TEMPLATES_DIR/$NEW_PDFF.pdf"

  exit 0
}

for arg in "$@"; do
  if [ "$arg" = "--new-document-templates" ]; then
    copy_templates
  fi
done

APP_PATH=/opt/M4_DESKTOPEDITORS_PREFIX
export LD_LIBRARY_PATH=$APP_PATH${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
exec $APP_PATH/DesktopEditors "$@"
