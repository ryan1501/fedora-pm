use anyhow::{anyhow, Result};
use regex::Regex;
use std::path::Path;

lazy_static::lazy_static! {
    static ref PACKAGE_NAME_REGEX: Regex = Regex::new(r"^[a-zA-Z0-9._+-]+$").unwrap();
    static ref REPO_NAME_REGEX: Regex = Regex::new(r"^[a-zA-Z0-9._-]+$").unwrap();
    static ref KERNEL_VERSION_REGEX: Regex = Regex::new(r"^\d+\.\d+\.\d+").unwrap();
    static ref SAFE_PATH_REGEX: Regex = Regex::new(r"^[a-zA-Z0-9/_.-]+$").unwrap();
}

pub fn validate_package_name(name: &str) -> Result<()> {
    if name.is_empty() {
        return Err(anyhow!("Package name cannot be empty"));
    }
    
    if name.len() > 100 {
        return Err(anyhow!("Package name too long (max 100 characters)"));
    }
    
    if !PACKAGE_NAME_REGEX.is_match(name) {
        return Err(anyhow!(
            "Invalid package name '{}'. Only alphanumeric characters, dots, underscores, hyphens and plus signs are allowed",
            name
        ));
    }
    
    // Check for suspicious patterns
    if name.contains("..") || name.starts_with('/') || name.starts_with('.') {
        return Err(anyhow!("Package name contains suspicious characters"));
    }
    
    Ok(())
}

pub fn validate_package_list(packages: &[String]) -> Result<()> {
    if packages.is_empty() {
        return Err(anyhow!("No packages specified"));
    }
    
    if packages.len() > 100 {
        return Err(anyhow!("Too many packages specified (max 100)"));
    }
    
    for package in packages {
        validate_package_name(package)?;
    }
    
    Ok(())
}

pub fn validate_repository_name(name: &str) -> Result<()> {
    if name.is_empty() {
        return Err(anyhow!("Repository name cannot be empty"));
    }
    
    if name.len() > 50 {
        return Err(anyhow!("Repository name too long (max 50 characters)"));
    }
    
    if !REPO_NAME_REGEX.is_match(name) {
        return Err(anyhow!(
            "Invalid repository name '{}'. Only alphanumeric characters, dots, underscores, and hyphens are allowed",
            name
        ));
    }
    
    Ok(())
}

pub fn validate_search_query(query: &str) -> Result<()> {
    if query.is_empty() {
        return Err(anyhow!("Search query cannot be empty"));
    }
    
    if query.len() > 200 {
        return Err(anyhow!("Search query too long (max 200 characters)"));
    }
    
    // Check for potentially dangerous shell commands
    let dangerous_patterns = [
        ";", "&&", "||", "|", "&", "$(", "`", "$", "${", ">", "<", ">>", "<<",
    ];
    
    for pattern in &dangerous_patterns {
        if query.contains(pattern) {
            return Err(anyhow!(
                "Search query contains potentially dangerous characters: '{}'",
                pattern
            ));
        }
    }
    
    Ok(())
}

pub fn validate_kernel_version(version: &str) -> Result<()> {
    if version.is_empty() {
        return Ok(()); // Empty means "latest"
    }
    
    if !KERNEL_VERSION_REGEX.is_match(version) {
        return Err(anyhow!(
            "Invalid kernel version format '{}'. Expected format: X.Y.Z",
            version
        ));
    }
    
    Ok(())
}

pub fn validate_file_path(path: &str) -> Result<()> {
    if path.is_empty() {
        return Err(anyhow!("File path cannot be empty"));
    }
    
    // Resolve relative path attempts
    if path.contains("..") {
        return Err(anyhow!("Relative path traversal not allowed"));
    }
    
    let path_obj = Path::new(path);
    
    // Check for safe characters
    if !SAFE_PATH_REGEX.is_match(path) {
        return Err(anyhow!(
            "File path contains unsafe characters: '{}'",
            path
        ));
    }
    
    // Check for dangerous extensions (if it's a file)
    if let Some(ext) = path_obj.extension() {
        let dangerous_exts = ["exe", "bat", "cmd", "sh", "scr", "vbs", "js"];
        if dangerous_exts.contains(&ext.to_string_lossy().to_lowercase().as_str()) {
            return Err(anyhow!(
                "Dangerous file extension '{}' not allowed",
                ext.to_string_lossy()
            ));
        }
    }
    
    Ok(())
}

pub fn validate_url(url: &str) -> Result<()> {
    if url.is_empty() {
        return Err(anyhow!("URL cannot be empty"));
    }
    
    if url.len() > 1000 {
        return Err(anyhow!("URL too long (max 1000 characters)"));
    }
    
    // Basic URL validation
    if !url.starts_with("http://") && !url.starts_with("https://") && !url.starts_with("ftp://") {
        return Err(anyhow!("URL must start with http://, https://, or ftp://"));
    }
    
    // Check for potentially dangerous URLs
    if url.contains("file://") || url.contains("data:") || url.contains("javascript:") {
        return Err(anyhow!("Unsafe URL protocol detected"));
    }
    
    Ok(())
}

pub fn validate_config_directory(path: &str) -> Result<()> {
    if path.is_empty() {
        return Err(anyhow!("Config directory path cannot be empty"));
    }
    
    let path_obj = Path::new(path);
    
    // Check for path traversal attempts
    if path_obj.components().any(|c| c.as_os_str() == "..") {
        return Err(anyhow!("Path traversal not allowed in config directory"));
    }
    
    // Check if parent directory exists (for validation)
    if let Some(parent) = path_obj.parent() {
        if !parent.exists() {
            return Err(anyhow!("Parent directory does not exist: {}", parent.display()));
        }
    }
    
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_validate_package_name() {
        assert!(validate_package_name("vim").is_ok());
        assert!(validate_package_name("package-1.0.0").is_ok());
        assert!(validate_package_name("gcc").is_ok());
        
        assert!(validate_package_name("").is_err());
        assert!(validate_package_name("package;rm -rf /").is_err());
        assert!(validate_package_name("../package").is_err());
    }
    
    #[test]
    fn test_validate_package_list() {
        assert!(validate_package_list(&["vim".to_string(), "git".to_string()]).is_ok());
        assert!(validate_package_list(&[]).is_err());
    }
    
    #[test]
    fn test_validate_search_query() {
        assert!(validate_search_query("python").is_ok());
        assert!(validate_search_query("").is_err());
        assert!(validate_search_query("python; rm -rf /").is_err());
    }
}