//
//  NavigationController.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import Foundation
import UIKit

extension UINavigationController {
    func navTitleWithImageAndText(titleText: String) -> UIView {
        let titleView = UIView()
        
        let label = UILabel()
        label.text = titleText
        label.sizeToFit()
        label.center = titleView.center
        label.textAlignment = .center
        
        let image = UIImageView()
        image.image = UIImage(named: "Logo")
        
        let imageAspect = image.image!.size.width / image.image!.size.height
        
        let imageX = label.frame.origin.x - label.frame.size.height * imageAspect - 5
        let imageY = label.frame.origin.y
        
        let imageW = label.frame.size.height * imageAspect
        let imageH = label.frame.size.height
        
        image.frame = CGRect(x: imageX, y: imageY, width: imageW, height: imageH)
        image.contentMode = .scaleAspectFit
        
        titleView.addSubview(label)
        titleView.addSubview(image)
        
        titleView.sizeToFit()
        
        return titleView
    }
}
