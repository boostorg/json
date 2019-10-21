<!-- INCLUDES_FOOT_TEMPLATE BEGIN -->
  <xsl:choose>
    <xsl:when test="contains($file, 'boost/json')">
      <xsl:text>&#xd;&#xd;Convenience header [include_file boost/json.hpp]&#xd;</xsl:text>
    </xsl:when>
  </xsl:choose>
<!-- INCLUDES_FOOT_TEMPLATE END -->
