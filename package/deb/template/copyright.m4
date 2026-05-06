Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: M4_COMPANY_NAME M4_PRODUCT_NAME
Upstream-Contact: M4_PUBLISHER_NAME <M4_SUPPORT_MAIL>
Source: M4_PUBLISHER_URL

Files: *
Copyright: 2009-M4_CURRENT_YEAR, M4_PUBLISHER_NAME <M4_SUPPORT_MAIL>
ifelse(M4_PACKAGE_EDITION, commercial,
`License: Proprietary',
`License: AGPL-3.0-only')

ifelse(M4_PACKAGE_EDITION, commercial,,
`Files: *.gif *.ico *.jpg *.png *.svg
Copyright: 2009-M4_CURRENT_YEAR, M4_PUBLISHER_NAME <M4_SUPPORT_MAIL>
License: CC-BY-SA-4.0')
